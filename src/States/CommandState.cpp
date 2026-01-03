/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CommandState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;
using Severity = Stm32ItmLogger::LoggerInterface::Severity;

Status CommandState::onEnter(const CommandEvent &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::onEnter(%s) command=%02x dataLength=%d\r\n",
                                     getMachine()->getName(), getName(), event.getName(), event.command,
                                     event.dataLength);

    stateEnteredMillis = millis();

    command = event.command;

    getMachine()->sendCommand(command, event.data, event.dataLength);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > CommandState::handle(const DataReceivedEvent &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const auto confirmation = getMachine()->rxData.data[0];
    getMachine()->lastConfirmationCode = Confirmation::Code{confirmation};

    const auto size = getMachine()->rxData.packetLength - 3;

    if (confirmation == 0x00) {
        Stm32Common::String::FixedString<50> str;
        for (int i = 0; i < size; i++) {
            str.printf("%02x", getMachine()->rxData.data[i + 1]);
        }

        log(Severity::NOTICE)->printf("CommandState(%02x): OK\r\n", command);

        return TransitionTo<ReadyState>{};
    }

    log(Severity::ERROR)->printf("CommandState(%02x): %s (%02x)\r\n", command,
                                 getMachine()->lastConfirmationCode.to_string(), getMachine()->lastConfirmationCode);


    return TransitionTo<ReadyState>{};
    // return DoNothing{};
}

/**
 * Handles the processing of a given LoopEvent in the CommandState.
 *
 * Checks if the current state has been active beyond a pre-defined timeout
 * (TIMEOUT_CMD). If the timeout is exceeded, a TimeoutEvent is enqueued into
 * the state machine.
 *
 * @param event The LoopEvent object received for processing.
 * @return A DoNothing action which signifies no specific transition occurs.
 */
DoNothing CommandState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

