/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CommandState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status CommandState::onEnter(const CommandEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s) command=%02x dataLength=%d\r\n",
                getMachine()->getName(), getName(), event.getName(), event.command, event.dataLength);

    stateEnteredMillis = millis();

    getMachine()->sendCommand(event.command, event.data, event.dataLength);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > CommandState::handle(const DataReceivedEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const auto confirmation = getMachine()->rxData.data[0];

    const auto size = getMachine()->rxData.packetLength - 3;

    Stm32Common::String::FixedString<50> str;
    for(int i=0; i < size; i++) {
        str.printf("%02x", getMachine()->rxData.data[i + 1]);
    }

    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
            ->printf("DATA (%d) : \"%s\"\r\n", size, str.c_str());



    if (confirmation == 0x00) {
        return TransitionTo<ReadyState>{};
    }

    return DoNothing{};
}

DoNothing CommandState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

