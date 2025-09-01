/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsWriteRegState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status PsWriteRegState::onEnter(const PsWriteRegEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s) address=%02x value=%02x\r\n",
                     getMachine()->getName(), getName(), event.getName(), event.address, event.value);

    stateEnteredMillis = millis();

    regAddress = event.address;
    regValue = event.value;

    const uint8_t data[2]{regAddress, regValue};

    // uint8_t data[4]{};
    // data[1] = regAddress;
    // data[3] = regValue;

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_WriteReg, data, sizeof(data));

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsWriteRegState::handle(const DataReceivedEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const auto confirmation = getMachine()->rxFrame[9];

    switch (confirmation) {
        case 0x00:
            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                    ->printf("PS_WriteReg(address=%02x value=%02x): OK\r\n", regAddress, regValue);
            return TransitionTo<ReadyState>{};

        case 0x01:
            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_WriteReg(address=%02x value=%02x): ERROR receiving the package\r\n",
                             regAddress, regValue);
            break;

        case 0x18:
            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_WriteReg(address=%02x value=%02x): ERROR reading/writing flash\r\n",
                             regAddress, regValue);
            break;

        case 0x1a:
            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_WriteReg(address=%02x value=%02x): ERROR incorrect register address\r\n",
                             regAddress, regValue);
            break;

        case 0x1b:
            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_WriteReg(address=%02x value=%02x): ERROR incorrect register value\r\n",
                             regAddress, regValue);
            break;

        default:
            log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_WriteReg(address=%02x value=%02x): ERROR\r\n",
                             regAddress, regValue);
            break;
    }

    return DoNothing{};
}

DoNothing PsWriteRegState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

