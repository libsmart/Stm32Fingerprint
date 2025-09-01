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

            if (std::strcmp(argv[1], "inf") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_ReadINFpage, nullptr, 0});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "sys") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_ReadSysPara, nullptr, 0});
                return runReturn::FINISHED;
            }

            if (std::strcmp(argv[1], "cancel") == 0) {
                fpSensor.enqueueEvent(CommandEvent{Stm32Fingerprint::SensorHiLinkZw0608::PS_Cancel, nullptr, 0});
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


#endif

            return runReturn::ERROR;
        }
    };
}
