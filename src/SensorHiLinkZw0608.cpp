/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "SensorHiLinkZw0608.hpp"
#include "StreamSession/GeneralStreamSession.hpp"

extern Stm32Common::StreamSession::Manager<Stm32Common::StreamSession::GeneralStreamSession, 1> fpSessionManager;

using namespace Stm32Fingerprint;

void SensorHiLinkZw0608::setup() {
    pinDetect.setup();
    pinEnable.setup();
    // pinEnable.setOn();
    serial.setup();
    ThreadXStateMachine::setup();
}

void SensorHiLinkZw0608::initialize() {
    enqueueEvent(InitializeEvent{});
}

void SensorHiLinkZw0608::loop() {
    pinDetect.loop();
    pinEnable.loop();
    serial.loop();
    ThreadXStateMachine::loop();
    parseReply();
    handle(LoopEvent{});
}

void SensorHiLinkZw0608::end() {
    ThreadXStateMachine::end();
    pinEnable.setOff();
}

void SensorHiLinkZw0608::errorHandler() {
    ThreadXStateMachine::errorHandler();
    pinEnable.setOff();
}

void SensorHiLinkZw0608::isrDetect() {
    enqueueEvent(DetectEvent{});
}

uint16_t SensorHiLinkZw0608::calc_checksum(uint8_t *data, int32_t start, int32_t end) {
    uint16_t chksum = 0;

    for (int32_t i = start; i < end; ++i) {
        chksum += (uint16_t) data[i];
    }

    return chksum;
}

bool SensorHiLinkZw0608::check_checksum(uint8_t *data, uint32_t len) {
    uint16_t chksum_to_check = 0;
    uint16_t chksum = 0;

    chksum_to_check = data[len - 1] + (data[len - 2] << 8);

    for (int32_t i = 6; i < len - 2; ++i)
        chksum += data[i];

    return (chksum_to_check == chksum);
}

void SensorHiLinkZw0608::parseReply() {
    const auto available = serial.available();

    if (available == 0) return;


    // auto rxBuffer = serial.getRxBuffer();
    const auto rxBuffer = fpSessionManager.getFirstSession()->getRxBuffer();
    const volatile auto buf = rxBuffer->getReadPointer();

    /*
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)->print("FP: ");
    for (size_t i = 0; i < available; i++) {
        log()->printf("%02x ", buf[i]);
    }
    log()->println();
    */

    size_t frame_length = 0;

    while (rxBuffer->available() > 0) {
        const char c = rxBuffer->read();
        rxFrame[frameBytesParsed++] = c;
        if (parserState != parserState_t::CHECKSUM) rollingChecksum += static_cast<uint16_t>(c);
        switch (parserState) {
            case parserState_t::NONE: {
                parserState = parserState_t::NONE;

                if (c == 0x55) {
                    // fp sensor sends 0x55
                    handle(InitOkReceivedEvent{});
                    frameBytesParsed = 0;
                    memset(rxFrame, 0, sizeof(rxFrame));
                    break;
                }

                if (c == 0xef) {
                    parserState = parserState_t::HEADER;
                    memset(&rxData, 0, sizeof(rxData));
                    rxData.header = c << 8;
                    cnt = 0;
                    break;
                }
            }

            case parserState_t::HEADER: {
                parserState = parserState_t::HEADER;

                if (c == 0x01) {
                    parserState = parserState_t::ADDRESS;
                    rxData.header |= c;
                    cnt = 0;
                }
                break;
            }

            case parserState_t::ADDRESS: {
                parserState = parserState_t::ADDRESS;

                rxData.address = (rxData.address << 8) | c;
                cnt++;
                if (cnt >= ADDRESS_SIZE) {
                    parserState = parserState_t::PACKAGE_ID;
                    cnt = 0;
                    rollingChecksum = 0;
                }
                break;
            }

            case parserState_t::PACKAGE_ID: {
                parserState = parserState_t::PACKAGE_ID;

                rxData.packageId = c;
                parserState = parserState_t::PACKET_LENGTH;
                cnt = 0;
                break;
            }

            case parserState_t::PACKET_LENGTH: {
                parserState = parserState_t::PACKET_LENGTH;

                rxData.packetLength = (rxData.packetLength << 8) | c;
                cnt++;

                if (cnt >= LENGTH_SIZE) {
                    frame_length = rxData.packetLength + DATA_OFFSET; //TODO: Check frame length
                    parserState = parserState_t::DATA;
                    cnt = 0;
                }
                break;
            }

            case parserState_t::DATA: {
                parserState = parserState_t::DATA;

                rxData.data = &rxFrame[DATA_OFFSET];
                cnt++;

                if (cnt >= rxData.packetLength - CHECKSUM_SIZE) {
                    parserState = parserState_t::CHECKSUM;
                    cnt = 0;
                }
                break;
            }

            case parserState_t::CHECKSUM: {
                parserState = parserState_t::CHECKSUM;

                rxData.checksum = (rxData.checksum << 8) | c;
                cnt++;
                if (cnt < CHECKSUM_SIZE) break;
            }

            case parserState_t::CHECK_CHECKSUM: {
                parserState = parserState_t::CHECK_CHECKSUM;

                if (rxData.checksum != rollingChecksum) {
                    parserState = parserState_t::ERROR;
                    break;
                }
            }

            case parserState_t::DONE: {
                parserState = parserState_t::DONE;

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Header         0x%04x\r\n", rxData.header);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Device address 0x%08x\r\n", rxData.address);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Package ID     0x%02x\r\n", rxData.packageId);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Package length 0x%04x (%d)\r\n", rxData.packetLength, rxData.packetLength);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("confirmation   0x%02x\r\n", rxData.data[0]);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Checksum       0x%04x (%d)\r\n", rxData.checksum, rxData.checksum);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)->print("RX: ");
                for (size_t i = 0; i < rxData.packetLength + DATA_OFFSET; i++) {
                    log()->printf("%02x ", rxFrame[i]);
                }
                log()->println();

                handle(DataReceivedEvent{});


                cnt = 0;
                frameBytesParsed = 0;
                memset(rxFrame, 0, sizeof(rxFrame));
                parserState = parserState_t::NONE;
                break;
            }

            case parserState_t::ERROR: {
                parserState = parserState_t::ERROR;
            }

            default: break;
        }
    }
}

void SensorHiLinkZw0608::sendCommand(const uint8_t instruction) {
    sendPacket(0x01, &instruction, 1);
}

void SensorHiLinkZw0608::sendCommand(const uint8_t instruction, const uint8_t *data, const uint16_t dataLength) {
    uint8_t tmpData[dataLength + 1];
    tmpData[0] = instruction;
    memcpy(tmpData + 1, data, dataLength);
    sendPacket(0x01, tmpData, dataLength + 1);
}

void SensorHiLinkZw0608::sendPacket(const uint8_t packetId, const uint8_t *data, const uint16_t dataLength) {
    const uint16_t frame_length = dataLength + 2 + 9;
    if (frame_length > sizeof(txFrame)) {
        throw std::runtime_error("txPacket too small");
    }

    auto *header = (uint16_t *) &txFrame[0];
    auto *device_address = (uint32_t *) &txFrame[2];
    auto *package_id = (uint8_t *) &txFrame[6];
    auto *package_length = (uint16_t *) &txFrame[7];
    auto *payload = (uint8_t *) &txFrame[9];
    auto *checksum = (uint16_t *) &txFrame[9 + dataLength];

    *header = __builtin_bswap16(0xef01);
    *device_address = __builtin_bswap32(address);
    *package_id = packetId;
    *package_length = __builtin_bswap16(dataLength + 2);

    if (data != nullptr) {
        memcpy(payload, data, dataLength);
    }

    *checksum = __builtin_bswap16(calc_checksum(reinterpret_cast<uint8_t *>(&txFrame), 6, frame_length - 2));

    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)->print("TX: ");
    for (size_t i = 0; i < frame_length; i++) {
        log()->printf("%02x ", txFrame[i]);
    }
    log()->println();

    serial.getSession()->write(reinterpret_cast<const uint8_t *>(&txFrame), frame_length);
}
