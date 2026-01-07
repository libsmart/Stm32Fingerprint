/*
 * SPDX-FileCopyrightText: 2026 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ReadyState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;
using Severity = Stm32ItmLogger::LoggerInterface::Severity;

Status ReadyState::onEnter(const EventInterface &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    getMachine()->flags.set(static_cast<ULONG>(SensorHiLinkZw0608::flags_t::READY));

    return Continue{};
}

DoNothing ReadyState::handle(const DetectEvent &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    return {};
}

DoNothing ReadyState::handle(const LoopEvent &event) {
    return {};
}

