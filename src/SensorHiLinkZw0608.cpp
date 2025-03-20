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
    auto rxBuffer = fpSessionManager.getFirstSession()->getRxBuffer();
    const volatile auto buf = rxBuffer->getReadPointer();

    // log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
    // ->printf("FP: %02x\r\n", rxBuffer->read());
    // return;


    if (available == 1 && buf[0] == 0x55) {
        // fp sensor sends 0x55
        handle(InitOkReceivedEvent{});
        rxBuffer->clear();
        return;
    }

    // if (available >= 11) {
    //     log()->print("FP:");
    //     for (int i = 0; i < available; i++) {
    //         log()->printf(" %02x", buf[i]);
    //     }
    //     log()->println();
    // } else {
    //     return;
    // }

    if (available >= 1 && buf[0] != 0xef) {
        rxBuffer->clear();
        return;
    }

    if (available >= 2 && buf[1] != 0x01) {
        rxBuffer->clear();
        return;
    }

    // check address


    // check for timeout

    // Packet length not yet received
    if (available < 9) { return; }
    const uint16_t packet_length = (buf[7] << 8) + buf[8];
    const uint16_t frame_length = packet_length + 9;

    // full packet received?
    if (available == frame_length) {
        if (check_checksum(const_cast<uint8_t *>(buf), rxBuffer->available())) {
            memcpy(&rxPacket, buf, frame_length);

            rxData.header = (rxPacket[0] << 8 | rxPacket[1]);
            rxData.address = (rxPacket[2] << 24 | rxPacket[3] << 16 | rxPacket[4] << 8 | rxPacket[5]);
            rxData.packageId = rxPacket[6];
            rxData.packetLength = (rxPacket[7] << 8 | rxPacket[8]);
            rxData.data = &rxPacket[9];
            rxData.checksum = (rxPacket[frame_length - 2] << 8 | rxPacket[frame_length - 1]);


            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("Header         0x%04x\r\n", rxData.header);

            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("Device address 0x%08x\r\n", rxData.address);

            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("Package ID     0x%02x\r\n", rxData.packageId);

            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("Package length 0x%04x (%d)\r\n", rxData.packetLength, rxData.packetLength);

            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("confirmation   0x%02x\r\n", rxData.data[0]);

            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("Checksum       0x%04x (%d)\r\n", rxData.checksum, rxData.checksum);

            handle(DataReceivedEvent{});
            rxBuffer->clear();
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
    if (frame_length > sizeof(txPacket)) {
        throw std::runtime_error("txPacket too small");
    }

    auto *header = (uint16_t *) &txPacket[0];
    auto *device_address = (uint32_t *) &txPacket[2];
    auto *package_id = (uint8_t *) &txPacket[6];
    auto *package_length = (uint16_t *) &txPacket[7];
    auto *payload = (uint8_t *) &txPacket[9];
    auto *checksum = (uint16_t *) &txPacket[9 + dataLength];

    *header = __builtin_bswap16(0xef01);
    *device_address = __builtin_bswap32(address);
    *package_id = packetId;
    *package_length = __builtin_bswap16(dataLength + 2);

    if (data != nullptr) {
        memcpy(payload, data, dataLength);
    }

    *checksum = __builtin_bswap16(calc_checksum(reinterpret_cast<uint8_t *>(&txPacket), 6, frame_length - 2));

    serial.getSession()->write(reinterpret_cast<const uint8_t *>(&txPacket), frame_length);
}
