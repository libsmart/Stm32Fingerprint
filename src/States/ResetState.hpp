/*
 * SPDX-FileCopyrightText: 2026 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "FSM/StateMachine.hpp"
#include "FSM/StateInterface.hpp"
#include "SensorEvents.hpp"
#include "SensorStates.hpp"

namespace Stm32Fingerprint {
    class SensorHiLinkZw0608;

    namespace States {
        struct ResetState
                : StateInterface<SensorHiLinkZw0608>,
                  Will<
                      ByDefault<DoNothing>,
                      On<ResetEvent, TransitionTo<ResetState> >
                  > {
            ResetState(const char *name, SensorHiLinkZw0608 *machine, Stm32ItmLogger::LoggerInterface *logger)
                : StateInterface(name, machine, logger) { ; }

            using Will::handle;

            Status onEnter(const EventInterface &event);
        };
    }
}
