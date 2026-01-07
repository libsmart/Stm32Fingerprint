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
        struct PsAutoEnrollState final
                : StateInterface<SensorHiLinkZw0608>,
                  Will<
                      ByDefault<DoNothing>,
                      On<TimeoutEvent, TransitionTo<PsCancelState> >,
                      On<ResetEvent, TransitionTo<ResetState> >,
                      On<ErrorEvent, TransitionTo<ResetState> >
                  > {
            PsAutoEnrollState(const char *name, SensorHiLinkZw0608 *machine, Stm32ItmLogger::LoggerInterface *logger)
                : StateInterface(name, machine, logger) { ; }

            Status onEnter(const PsAutoEnrollEvent &event);

            OneOf<DoNothing, TransitionTo<ReadyState>> handle(const DataReceivedEvent &event);

            DoNothing handle(const LoopEvent &event);

            using Will::handle;

        private:
            uint32_t stateEnteredMillis = 0;
            static constexpr uint32_t TIMEOUT_CMD = LIBSMART_SECONDS_TO_MS(5);
            const AutoEnrollResult *result{};
            void restartTimeout();
        };
    }
}
