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

            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                    ->printf("confirmation = 0x%02x\r\n", buf[9]);

            handle(DataReceivedEvent{});
            rxBuffer->clear();
        }
    }
}

void SensorHiLinkZw0608::sendCommand(uint8_t instruction) {
    sendPacket(0x01, &instruction, 1);
}

void SensorHiLinkZw0608::sendPacket(uint8_t packetId, uint8_t *data, uint16_t dataLength) {

    const uint16_t frame_length = dataLength + 2 + 9;
    if(frame_length > sizeof(txPacket)) {
        throw std::runtime_error("txPacket too small");
    }

    auto *header = (uint16_t *)&txPacket[0];
    auto *device_address = (uint32_t *)&txPacket[2];
    auto *package_id = (uint8_t *)&txPacket[6];
    auto *package_length = (uint16_t *)&txPacket[7];
    auto *payload = (uint8_t *)&txPacket[9];
    auto *checksum = (uint16_t *)&txPacket[9 + dataLength];

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
