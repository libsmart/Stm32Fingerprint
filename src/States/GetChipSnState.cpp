/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "GetChipSnState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status GetChipSnState::onEnter(const GetChipSnEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    stateEnteredMillis = millis();

    const uint8_t data[1]{};
    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_GetChipSN, data, 1);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > GetChipSnState::handle(const DataReceivedEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const auto confirmation = getMachine()->rxPacket[9];

    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
            ->printf("serial %02x%02x%02x%02x\r\n",
                     getMachine()->rxPacket[10], getMachine()->rxPacket[11],
                     getMachine()->rxPacket[12], getMachine()->rxPacket[13]);

    if (confirmation == 0x00) {
        return TransitionTo<ReadyState>{};
    }

    return DoNothing{};
}

DoNothing GetChipSnState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > 500) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

