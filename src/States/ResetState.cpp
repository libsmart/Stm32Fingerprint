/*
 * SPDX-FileCopyrightText: 2026 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ResetState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;
using Severity = Stm32ItmLogger::LoggerInterface::Severity;

Status ResetState::onEnter(const EventInterface &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    getMachine()->cancel();
    // getMachine()->pinEnable.setOff();

    getMachine()->transitionTo<ReadyState>();

    return Continue{};
}

