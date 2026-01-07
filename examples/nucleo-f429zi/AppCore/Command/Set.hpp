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
            using namespace Stm32ItmLogger;
            using Severity = LoggerInterface::Severity;

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

            if (std::strcmp(argv[1], "debug") == 0) {
                if (argc == 3) {
                    auto debugLevel = static_cast<uint8_t>(strtoll(argv[2], nullptr, DEC));
                    out()->printf("DEBUG LEVEL: %d\r\n", debugLevel);
                    Logger.setPrintSeverity(static_cast<LoggerInterface::Severity>(debugLevel));
                }
                const long sev = static_cast<long>(Logger.getPrintSeverity());
                out()->printf("DEBUG LEVEL: %d\r\n", sev);
                out()->print("INFO: EMERGENCY (1)      ");
                out()->println(sev & static_cast<uint8_t>(Severity::EMERGENCY) ? "ON" : "OFF");
                out()->print("INFO: ALERT (2)          ");
                out()->println(sev & static_cast<uint8_t>(Severity::ALERT) ? "ON" : "OFF");
                out()->print("INFO: CRITICAL (4)       ");
                out()->println(sev & static_cast<uint8_t>(Severity::CRITICAL) ? "ON" : "OFF");
                out()->print("INFO: ERROR (8)          ");
                out()->println(sev & static_cast<uint8_t>(Severity::ERROR) ? "ON" : "OFF");
                out()->print("INFO: WARNING (16)       ");
                out()->println(sev & static_cast<uint8_t>(Severity::WARNING) ? "ON" : "OFF");
                out()->print("INFO: NOTICE (32)        ");
                out()->println(sev & static_cast<uint8_t>(Severity::NOTICE) ? "ON" : "OFF");
                out()->print("INFO: INFORMATIONAL (64) ");
                out()->println(sev & static_cast<uint8_t>(Severity::INFORMATIONAL) ? "ON" : "OFF");
                out()->print("INFO: DEBUGGING (128)    ");
                out()->println(sev & static_cast<uint8_t>(Severity::DEBUGGING) ? "ON" : "OFF");
                out()->flush();
                return runReturn::FINISHED;
            }


            return runReturn::ERROR;
        }
    };
}
