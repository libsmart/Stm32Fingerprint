/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include "ezShell/Shell.hpp"
#include "ezShell/Command/Info.hpp"

namespace AppCore::Command {
    class Info : public Stm32Shell::ezShell::Command::Info {
    public:
        runReturn run() override {
            auto ret = Stm32Shell::ezShell::Command::Info::run();

            OUTPUT_PAUSE;

            return ret;
        }
    };
}
