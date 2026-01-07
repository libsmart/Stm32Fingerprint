/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

namespace Stm32Fingerprint::States {
    struct UninitializedState;
    struct InitializeState;
    struct ReadyState;
    struct CommandState;
    struct PsCancelState;
    struct PsUpCharState;
    struct PsUpImageState;
    struct PsDownImageState;
    struct PsReadSysParaState;
    struct PsReadInfPageState;
    struct PsAutoIdentifyState;
    struct PsAutoEnrollState;
    struct GetChipSnState;
    struct HandShakeState;
    struct ResetState;
    struct ErrorState;
}

#include "States/UninitializedState.hpp"
#include "States/InitializeState.hpp"
#include "States/ReadyState.hpp"
#include "States/CommandState.hpp"
#include "States/PsCancelState.hpp"
#include "States/PsUpCharState.hpp"
#include "States/PsUpImageState.hpp"
#include "States/PsDownImageState.hpp"
#include "States/PsReadSysParaState.hpp"
#include "States/PsReadInfPageState.hpp"
#include "States/PsAutoIdentifyState.hpp"
#include "States/PsAutoEnrollState.hpp"
#include "States/GetChipSnState.hpp"
#include "States/HandShakeState.hpp"
#include "States/ResetState.hpp"
#include "States/ErrorState.hpp"
