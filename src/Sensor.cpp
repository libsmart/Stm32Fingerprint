/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Sensor.hpp"

void Stm32Fingerprint::Sensor::setup() {
    if (currentState == STATE_UNINITIALIZED) {
        pinDetect->setup();
        pinEnable->setup();
        pinEnable->setOn();
        serial->setup();
        currentState = STATE_SETUP;
        lastStateChange = millis();
    }
}

void Stm32Fingerprint::Sensor::loop() {
    pinDetect->loop();
    pinEnable->loop();
    serial->loop();


    if (currentState != previousState) {
        log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
                ->printf("%s: %d => %d\r\n", getName(), previousState, currentState);
        previousState = currentState;
    }

    switch (currentState) {
        case STATE_UNINITIALIZED:
            return;

        case STATE_SETUP:
            if (millis() - lastStateChange <= LIBSMART_STM32FINGERPRINT_SETUP_WAIT_MS) return;
            lastStateChange = 0;
            currentState = pinEnable->isOn() ? STATE_STANDBY_READY_WAIT : STATE_STANDBY;
            return;

        case STATE_STANDBY:
            currentState = STATE_STANDBY;
            pinEnable->setOff();
            return;

        case STATE_STANDBY_READY_WAIT:
            currentState = STATE_STANDBY_READY_WAIT;
            pinEnable->setOn();
            parseReply();
            if (lastStateChange == 0) lastStateChange = millis();
            if (millis() - lastStateChange <= LIBSMART_STM32FINGERPRINT_READY_WAIT_MS) return;
            lastStateChange = 0;

        case STATE_READY:
            currentState = STATE_READY;
            parseReply();
            if (lastStateChange == 0) lastStateChange = millis();
            if (millis() - lastStateChange <= LIBSMART_STM32FINGERPRINT_STANDBY_TIMEOUT_MS) return;
            lastStateChange = 0;
            standby();
    }
}

void Stm32Fingerprint::Sensor::end() {
    pinEnable->setOff();
}

void Stm32Fingerprint::Sensor::errorHandler() {
    pinEnable->setOff();
}

void Stm32Fingerprint::Sensor::parseReply() {
    const auto available = serial->available();

    if (available == 0) return;

    auto rxBuffer = serial->getRxBuffer();
    volatile auto buf = rxBuffer->getReadPointer();


    if (available == 1 && buf[0] != 0x55) {
        // fp sensor sends 0x55
        rxBuffer->clear();
        currentState = STATE_READY;
        lastStateChange = 0;
        return;
    }


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
            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                    ->printf("confirmation = 0x%02x\r\n", buf[9]);
            rxBuffer->clear();
        }
    }
}

void Stm32Fingerprint::Sensor::isrDetect() {
    if (currentState == STATE_STANDBY) enable();
}

void Stm32Fingerprint::Sensor::enable() {
    pinEnable->setOn();
    currentState = STATE_STANDBY_READY_WAIT;
}

void Stm32Fingerprint::Sensor::standby() {
    pinEnable->setOff();
    currentState = STATE_STANDBY;
}

void Stm32Fingerprint::Sensor::handShake() {
#pragma pack(push, 1)
    struct data_t {
        uint16_t header = __builtin_bswap16(0xef01);
        uint32_t module_address = __builtin_bswap32(0xffffffff);
        uint8_t package_identifier = 0x01;
        uint16_t package_length = 0;
        uint8_t instruction_code = 0x35;
        uint16_t checksum = 0;
    };
    data_t data;
#pragma pack(pop)

    data.package_length = __builtin_bswap16(sizeof(data) - 9);
    data.checksum = __builtin_bswap16(checksum(reinterpret_cast<uint8_t *>(&data), 6, sizeof(data) - 2));

    serial->write(reinterpret_cast<const uint8_t *>(&data), sizeof(data));
    // HAL_UART_Transmit(huart, reinterpret_cast<uint8_t *>(&data), sizeof(data), 100);
    // uint8_t buf[50]={};
    // HAL_UART_Receive(huart, buf, sizeof(buf), 100);

    Stm32ItmLogger::logger.println();
}

void Stm32Fingerprint::Sensor::readSysPara() {
#pragma pack(push, 1)
    struct data_t {
        uint16_t header = 0xef01;
        uint32_t module_address = 0xffffffff;
        uint8_t package_identifier = 0x01;
        uint16_t package_length = 0x03;
        uint8_t instruction_code = 0x0f;
        uint16_t checksum = 0;
    };
    data_t data;
#pragma pack(pop)

    data.checksum = checksum(reinterpret_cast<uint8_t *>(&data), 0, sizeof(data) - 2);

    serial->write(reinterpret_cast<const uint8_t *>(&data), sizeof(data));
    // HAL_UART_Transmit(huart, reinterpret_cast<uint8_t *>(&data), sizeof(data), 100);
    // uint8_t buf[50]={};
    // HAL_UART_Receive(huart, buf, sizeof(buf), 100);

    Stm32ItmLogger::logger.println();
}

uint16_t Stm32Fingerprint::Sensor::checksum(uint8_t *data, int32_t start, int32_t end) {
    uint16_t chksum = 0;

    for (int32_t i = start; i < end; ++i) {
        chksum += (uint16_t) data[i];
    }

    return chksum;
}

bool Stm32Fingerprint::Sensor::check_checksum(uint8_t *data, uint32_t len) {
    uint16_t chksum_to_check = 0;
    uint16_t chksum = 0;

    chksum_to_check = data[len - 1] + (data[len - 2] << 8);

    for (int32_t i = 6; i < len - 2; ++i)
        chksum += data[i];

    return (chksum_to_check == chksum);
}

