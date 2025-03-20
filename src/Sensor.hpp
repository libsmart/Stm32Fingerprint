/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LIBSMART_STM32FINGERPRINT_SENSOR_HPP
#define LIBSMART_STM32FINGERPRINT_SENSOR_HPP

#include <libsmart_config.hpp>
#include <main.h>

#include "Loggable.hpp"
#include "Nameable.hpp"
#include "PinDigitalIn.hpp"
#include "PinDigitalOut.hpp"
#include "Stm32Serial.hpp"
#include "Process/ProcessInterface.hpp"

namespace Stm32Fingerprint {
    class Sensor : public Stm32Common::Process::ProcessInterface,
                   public Stm32ItmLogger::Loggable,
                   public Stm32Common::Nameable {
    public:
        Sensor(const char *name, Stm32Serial::Stm32Serial *serial, Stm32Gpio::PinDigitalIn *pin_detect,
               Stm32Gpio::PinDigitalOut *pin_n_stdby, Stm32ItmLogger::LoggerInterface *logger)
            : Loggable(logger), Nameable(name), pinDetect(pin_detect), pinEnable(pin_n_stdby), serial(serial) {
        }


        using state = enum state {
            STATE_UNINITIALIZED, STATE_SETUP, STATE_STANDBY, STATE_STANDBY_READY_WAIT, STATE_READY
        };

        void setup() override;

        void loop() override;

        void end() override;

        void errorHandler() override;

        void parseReply();

        void isrDetect();

        void enable();
        void standby();

        void handShake();
        void readSysPara();

    protected:
        uint16_t checksum(uint8_t *data, int32_t start, int32_t end);
        bool check_checksum(uint8_t *data, uint32_t len);


    private:
        Stm32Gpio::PinDigitalIn *pinDetect{};
        Stm32Gpio::PinDigitalOut *pinEnable{};
        state currentState = STATE_UNINITIALIZED;
        state previousState = static_cast<state>(-1);
        uint32_t lastStateChange = 0;
        Stm32Serial::Stm32Serial *serial;
        uint32_t address = 0xffffffff;
    };
}

#endif
