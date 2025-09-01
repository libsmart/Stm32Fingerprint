/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "FSM/StateMachine.hpp"
#include "FSM/StateInterface.hpp"
#include "SensorEvents.hpp"
#include "SensorStates.hpp"

using namespace AppCore::FSM;
using namespace Stm32Fingerprint::Events;

namespace Stm32Fingerprint {
    class SensorHiLinkZw0608;

    namespace States {
        struct CommandState
                : StateInterface<SensorHiLinkZw0608>,
                  Will<
                      ByDefault<DoNothing>,
                      On<TimeoutEvent, TransitionTo<ReadyState> >,
                      On<ResetEvent, TransitionTo<ResetState> >
                  > {
            CommandState(const char *name, SensorHiLinkZw0608 *machine, Stm32ItmLogger::LoggerInterface *logger)
                : StateInterface(name, machine, logger) { ; }

            using Will::handle;

            Status onEnter(const CommandEvent &event);

            OneOf<DoNothing, TransitionTo<ReadyState>> handle(const DataReceivedEvent &event);

            DoNothing handle(const LoopEvent &event);

        private:
            uint32_t stateEnteredMillis = 0;
            static constexpr uint32_t TIMEOUT_CMD = 1000;

            uint8_t command = 0;
        };
    }
}
