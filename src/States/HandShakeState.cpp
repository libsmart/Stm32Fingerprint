/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "HandShakeState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status HandShakeState::onEnter(const HandShakeEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    stateEnteredMillis = millis();

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_HandShake);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > HandShakeState::handle(const DataReceivedEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    const auto confirmation = getMachine()->rxFrame[9];
    getMachine()->lastConfirmationCode = static_cast<Confirmation::Code>(confirmation);

    if (confirmation == 0x00) {
        return TransitionTo<ReadyState>{};
    }

    return DoNothing{};
}

DoNothing HandShakeState::handle(const LoopEvent &event) {
    if(millis() - stateEnteredMillis > 500) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

