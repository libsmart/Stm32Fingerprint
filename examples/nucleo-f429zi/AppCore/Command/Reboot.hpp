/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include "Command/AbstractCommand.hpp"
#include "ezShell/Shell.hpp"

using namespace Stm32ItmLogger;

namespace AppCore::Command {
    class Reboot : public Stm32Shell::Command::AbstractCommand {
    public:
        Reboot() {
            Nameable::setName("reboot");
            isSync = true;
            setLogger(&Logger);
        }


        runReturn run() override {
            auto ret = AbstractCommand::run();

            out()->println("REBOOTING...");
            out()->println();
            delay(500);

            HAL_NVIC_SystemReset();

            return ret;
        }
    };
}
