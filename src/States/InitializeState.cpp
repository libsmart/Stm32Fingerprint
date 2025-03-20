/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "InitializeState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status InitializeState::onEnter(const InitializeEvent &event) {
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    getMachine()->pinEnable.setOn();
    stateEnteredMillis = millis();

    return Continue{};
}

DoNothing InitializeState::handle(const LoopEvent &event) {
    // if (millis() - stateEnteredMillis > LIBSMART_STM32FINGERPRINT_INITIALIZE_WAIT_MS) {
    // getMachine()->enqueueEvent(HandShakeEvent{});
    // }

    return {};
}

