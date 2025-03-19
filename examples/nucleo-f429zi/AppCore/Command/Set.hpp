/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include "globals.hpp"
#include "Command/AbstractCommand.hpp"
#include "ezShell/Shell.hpp"

namespace AppCore::Command {
    class Set : public Stm32Shell::Command::AbstractCommand {
    public:
        Set() {
            Nameable::setName("set");
            isSync = true;
            setLogger(&Logger);
        }

        runReturn run() override {
            auto ret = AbstractCommand::run();

            if (argc == 3) {
                out()->printf("%s = %s\r\n", argv[1], argv[2]);

                if (std::strcmp(argv[1], "endpoint") == 0) {
                    // webApi.setEndpointUrl(argv[2]);
                    return runReturn::FINISHED;
                }

                if (std::strcmp(argv[1], "terminal-name") == 0) {
                    // webApi.setTerminalName(argv[2]);
                    return runReturn::FINISHED;
                }

                if (std::strcmp(argv[1], "time") == 0) {
                    // Stm32Rtc::DateTimeType dateTime;

                    // uint32_t timestamp = 0;
                    // timestamp = strtoull(argv[2], nullptr, DEC);
                    // dateTime.setTimestamp(timestamp);
                    // rtc.setDateTime(&dateTime);

                    return runReturn::FINISHED;
                }

            }

            return runReturn::ERROR;
        }
    };
}
