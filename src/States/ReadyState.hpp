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
        struct ReadyState
                : StateInterface<SensorHiLinkZw0608>,
                  Will<
                      ByDefault<DoNothing>,
                      On<CommandEvent, TransitionTo<CommandState> >,
                      On<PsCancelEvent, TransitionTo<PsCancelState> >,
                      On<PsUpCharEvent, TransitionTo<PsUpCharState> >,
                      On<PsUpImageEvent, TransitionTo<PsUpImageState> >,
                      On<PsDownImageEvent, TransitionTo<PsDownImageState> >,
                      On<PsReadSysParaEvent, TransitionTo<PsReadSysParaState> >,
                      On<PsReadInfPageEvent, TransitionTo<PsReadInfPageState> >,
                      On<PsAutoIdentifyEvent, TransitionTo<PsAutoIdentifyState> >,
                      On<PsAutoEnrollEvent, TransitionTo<PsAutoEnrollState> >,
                      On<GetChipSnEvent, TransitionTo<GetChipSnState> >,
                      On<HandShakeEvent, TransitionTo<HandShakeState> >,
                      On<ResetEvent, TransitionTo<ResetState> >
                  > {
            ReadyState(const char *name, SensorHiLinkZw0608 *machine, Stm32ItmLogger::LoggerInterface *logger)
                : StateInterface(name, machine, logger) { ; }

            Status onEnter(const EventInterface &event);

            DoNothing handle(const DetectEvent &event);

            DoNothing handle(const LoopEvent &event);

            using Will::handle;

        private:
            uint32_t stateEnteredMillis = 0;
        };
    }
}
