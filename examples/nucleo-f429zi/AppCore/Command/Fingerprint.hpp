/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include "globals.hpp"
#include "Command/AbstractCommand.hpp"
#include "ezShell/Shell.hpp"

namespace AppCore::Command {
    class Fingerprint : public Stm32Shell::Command::AbstractCommand {
    public:
        Fingerprint() {
            Nameable::setName("fp");
            isSync = true;
            setLogger(&Logger);
        }

        runReturn run() override {
            auto ret = AbstractCommand::run();

#if ENABLE_FP==1
            if (std::strcmp(argv[1], "info") == 0) {
                out()->println("Fingerprint info");
                out()->printf("fp_nSTDBY : %s\r\n", fp_nSTDBY.isOn() ? "ON" : "OFF");
                out()->printf("fp_DETECT : %s\r\n", fp_DETECT.isOn() ? "ON" : "OFF");

                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "on") == 0) {
                fp_nSTDBY.setOn();
                return runReturn::FINISHED;
            }
            if (std::strcmp(argv[1], "off") == 0) {
                fp_nSTDBY.setOff();
                return runReturn::FINISHED;
            }
            if (std::strcmp(argv[1], "start") == 0) {
                fpSensor.enqueueEvent(HandShakeEvent{});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "sn") == 0) {
                fpSensor.enqueueEvent(GetChipSnEvent{});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "writereg") == 0) {
                if (argc != 4) {
                    return runReturn::ERROR;
                }
                const auto address = static_cast<uint8_t>(std::strtoul(argv[2], nullptr, DEC));
                const auto value = static_cast<uint8_t>(std::strtoul(argv[3], nullptr, DEC));
                fpSensor.enqueueEvent(PsWriteRegEvent{address, value});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "inf") == 0) {
                fpSensor.enqueueEvent(PsReadInfPageEvent{});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "sys") == 0) {
                fpSensor.enqueueEvent(PsReadSysParaEvent{});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "cancel") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_Cancel, nullptr, 0});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "empty") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_Empty, nullptr, 0});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "restsetting") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_RestSetting, nullptr, 0});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "gi") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_GetImage, nullptr, 0});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "ui") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_UpImage, nullptr, 0});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "gc") == 0) {
                constexpr uint8_t data[1]={1};
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_GenChar, data, sizeof(data)});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "uc") == 0) {
                constexpr uint8_t data[1]={2};
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_UpChar, data, sizeof(data)});
                return runReturn::FINISHED;
            }


#endif

            return runReturn::ERROR;
        }
    };
}
