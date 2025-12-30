/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsReadInfPageState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status PsReadInfPageState::onEnter(const PsReadInfPageEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    stateEnteredMillis = millis();

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_ReadINFpage, nullptr, 0);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsReadInfPageState::handle(const DataReceivedEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const auto packageId = getMachine()->rxFrame[6];
    const auto confirmation = getMachine()->rxFrame[9];
    getMachine()->lastConfirmationCode = static_cast<Confirmation::Code>(confirmation);


    constexpr auto be16 = [](const uint8_t *p) -> uint16_t {
        return (static_cast<uint16_t>(p[0]) << 8) | static_cast<uint16_t>(p[1]);
    };
    constexpr auto be32 = [](const uint8_t *p) -> uint32_t {
        return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
    };

    if (packageId == 0x07 && confirmation == 0x00) {
        log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                ->printf("PS_ReadINFpage(): OK\r\n");
        // Data package follows
        return DoNothing{};
    }

    if (packageId == 0x08 /*&& confirmation == 0x00*/) {
        log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE);
        constexpr size_t BASE = 9;
        log()->printf(" enroll times      : %d\r\n", be16(&getMachine()->rxFrame[BASE + 0]));
        log()->printf(" template size     : %d\r\n", be16(&getMachine()->rxFrame[BASE + 2]));
        log()->printf(" database capacity : %d\r\n", be16(&getMachine()->rxFrame[BASE + 4]));
        log()->printf(" score level       : %d\r\n", be16(&getMachine()->rxFrame[BASE + 6]));
        log()->printf(" device address    : %08x\r\n", be32(&getMachine()->rxFrame[BASE + 8]));
        log()->printf(" packet size       : %d\r\n", be16(&getMachine()->rxFrame[BASE + 12]));
        log()->printf(" baud rate         : %d\r\n", be16(&getMachine()->rxFrame[BASE + 14]) * 9600); // 7
        log()->printf(" Anti-fake fingerp : %d\r\n", be16(&getMachine()->rxFrame[BASE + 16])); // 8
        log()->printf(" sensor parameters : %d\r\n", be16(&getMachine()->rxFrame[BASE + 18])); // 9
        log()->printf(" encryption level  : %d\r\n", be16(&getMachine()->rxFrame[BASE + 20])); // 10
        log()->printf(" enroll logic      : %d\r\n", be16(&getMachine()->rxFrame[BASE + 22])); // 11
        log()->printf(" image format      : %d\r\n", be16(&getMachine()->rxFrame[BASE + 24])); // 12
        log()->printf(" serial port delay : %d\r\n", be16(&getMachine()->rxFrame[BASE + 26])); // 13
        log()->printf(" Product s/n       : %.*s\r\n", 8, (const char *)&getMachine()->rxFrame[BASE + 28]); // 14
        log()->printf(" Software version  : %.*s\r\n", 8, (const char *)&getMachine()->rxFrame[BASE + 36]); // 15
        log()->printf(" Manufacturer      : %.*s\r\n", 8, (const char *)&getMachine()->rxFrame[BASE + 44]); // 16
        log()->printf(" Sensor name       : %.*s\r\n", 8, (const char *)&getMachine()->rxFrame[BASE + 52]); // 17
        log()->printf(" password          : %08x\r\n", be32(&getMachine()->rxFrame[BASE + 60])); // 18
        log()->printf(" JTAG lock flag    : %08x\r\n", be32(&getMachine()->rxFrame[BASE + 64])); // 19
        log()->printf(" res               : %04x\r\n", be16(&getMachine()->rxFrame[BASE + 68])); // 20
        log()->printf(" res               : %04x\r\n", be16(&getMachine()->rxFrame[BASE + 70])); // 21
        log()->printf(" res               : %04x\r\n", be16(&getMachine()->rxFrame[BASE + 72])); // 22
        log()->printf(" Valid flag        : %04x\r\n", be16(&getMachine()->rxFrame[BASE + 134])); // 23


        log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                ->printf("PS_ReadINFpage(): OK\r\n");
        return TransitionTo<ReadyState>{};
    }

    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
            ->printf("PS_ReadINFpage(): ERROR %02x\r\n", confirmation);

    return DoNothing{};
}

DoNothing PsReadInfPageState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

