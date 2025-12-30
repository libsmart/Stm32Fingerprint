/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsReadSysParaState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status PsReadSysParaState::onEnter(const PsReadSysParaEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    stateEnteredMillis = millis();

    sysPara = event.sysPara;

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_ReadSysPara, nullptr, 0);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsReadSysParaState::handle(const DataReceivedEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const auto confirmation = getMachine()->rxFrame[9];
    getMachine()->lastConfirmationCode = static_cast<Confirmation::Code>(confirmation);


    constexpr auto be16 = [](const uint8_t *p) -> uint16_t {
        return (static_cast<uint16_t>(p[0]) << 8) | static_cast<uint16_t>(p[1]);
    };
    constexpr auto be32 = [](const uint8_t *p) -> uint32_t {
        return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
    };


    if (confirmation == 0x00) {
        log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                ->printf("PS_ReadSysPara(): OK\r\n");

        if (sysPara != nullptr) {
            sysPara->number = be16(&getMachine()->rxFrame[10]);
            sysPara->templateSize = be16(&getMachine()->rxFrame[12]);
            sysPara->databaseCapacity = be16(&getMachine()->rxFrame[14]);
            sysPara->scoreLevelCode = be16(&getMachine()->rxFrame[16]);
            sysPara->deviceAddress = be32(&getMachine()->rxFrame[18]);
            sysPara->packetSize = be16(&getMachine()->rxFrame[22]);
            sysPara->baudRate = be16(&getMachine()->rxFrame[24]) * 9600;
        }

        log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE);
        log()->printf(" 1                 : %d\r\n", be16(&getMachine()->rxFrame[10]));
        log()->printf(" template size     : %d\r\n", be16(&getMachine()->rxFrame[12]));
        log()->printf(" database capacity : %d\r\n", be16(&getMachine()->rxFrame[14]));
        log()->printf(" score level       : %d\r\n", be16(&getMachine()->rxFrame[16]));
        log()->printf(" device address    : %08x\r\n", be32(&getMachine()->rxFrame[18]));
        log()->printf(" packet size       : %d\r\n", be16(&getMachine()->rxFrame[22]));
        log()->printf(" baud rate         : %d\r\n", be16(&getMachine()->rxFrame[24]) * 9600);

        return TransitionTo<ReadyState>{};
    }

    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
            ->printf("PS_ReadSysPara(): ERROR\r\n");

    return DoNothing{};
}

DoNothing PsReadSysParaState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

