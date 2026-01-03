/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef NUCLEO_F429ZI_APPCORE_COMMAND_REGISTERCOMMANDS_HPP
#define NUCLEO_F429ZI_APPCORE_COMMAND_REGISTERCOMMANDS_HPP

#include "Fingerprint.hpp"
#include "Info.hpp"
#include "Set.hpp"
#include "Reboot.hpp"
#include "ezShell/Shell.hpp"
#include "ezShell/Command/Help.hpp"

using namespace Stm32Shell::ezShell;

namespace AppCore::Command {
    inline ::Command::Help help;
    inline Info info;
    inline Set set;
    inline Reboot reboot;
    inline Fingerprint fingerprint;

    /**
     * @class RegisterCommands
     *
     * @brief The RegisterCommands class is responsible for managing the registration
     * of commands within the application.
     *
     * The RegisterCommands class provides an interface for adding, removing, and
     * managing command objects. It ensures that commands are properly registered
     * and accessible when needed.
     */
    class RegisterCommands {
    public:
        void operator()() const {
            Shell::registerCmd(&info);
            Shell::registerCmd(&help);
            Shell::registerCmd(&set);
            Shell::registerCmd(&reboot);
            Shell::registerCmd(&fingerprint);
        }
    };
}
#endif
