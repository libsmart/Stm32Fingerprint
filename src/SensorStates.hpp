/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

namespace Stm32Fingerprint::States {
    struct UninitializedState;
    struct InitializeState;
    struct ReadyState;
    struct HandShakeState;
    struct ResetState;
    struct ErrorState;
}

#include "States/UninitializedState.hpp"
#include "States/InitializeState.hpp"
#include "States/ReadyState.hpp"
#include "States/HandShakeState.hpp"
#include "States/ResetState.hpp"
#include "States/ErrorState.hpp"
