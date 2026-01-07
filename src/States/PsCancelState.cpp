/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsCancelState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;
using Severity = Stm32ItmLogger::LoggerInterface::Severity;

Status PsCancelState::onEnter(const PsCancelEvent &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    stateEnteredMillis = millis();
    ignoreConfirmation = false;
    getMachine()->sendCommand(SensorHiLinkZw0608::PS_Cancel);

    return Continue{};
}

Status PsCancelState::onEnter(const EventInterface &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    stateEnteredMillis = millis();
    ignoreConfirmation = true;
    getMachine()->sendCommand(SensorHiLinkZw0608::PS_Cancel);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsCancelState::handle(const DataReceivedEvent &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const Confirmation confirmation = Confirmation::Code{getMachine()->rxData.data[0]};
    if (!ignoreConfirmation) getMachine()->lastConfirmationCode = confirmation;

    if (confirmation == Confirmation::Code::OK) {
        log(Severity::NOTICE)->printf("PsCancelState(): OK\r\n");
        return TransitionTo<ReadyState>{};
    }

    log(Severity::ERROR)->printf("PsCancelState(): %s (%02x)\r\n",
                                 confirmation.to_string(), confirmation);


    return TransitionTo<ReadyState>{};
}

/**
 * Handles the processing of a given LoopEvent in the PsCancelState.
 *
 * Checks if the current state has been active beyond a pre-defined timeout
 * (TIMEOUT_CMD). If the timeout is exceeded, a TimeoutEvent is enqueued into
 * the state machine.
 *
 * @param event The LoopEvent object received for processing.
 * @return A DoNothing action which signifies no specific transition occurs.
 */
DoNothing PsCancelState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        log(Severity::ERROR)->printf("PsCancelState(): TIMEOUT\r\n");
        if (!ignoreConfirmation) getMachine()->lastConfirmationCode = Confirmation::Code::ERROR_TIMEOUT;
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

