/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "globals.hpp"
#include "Command/AbstractCommand.hpp"
#include "ezShell/Shell.hpp"
#include "Hash/Fnv1a.hpp"
#include "Hash/MurmurHash3.hpp"

namespace AppCore::Command {
    class Fingerprint : public Stm32Shell::Command::AbstractCommand {
    public:
        Fingerprint() {
            Nameable::setName("fp");
            isSync = true;
            setLogger(&Logger);
        }

        uint8_t data[36864]{};

        runReturn run() override {
            using namespace Stm32Fingerprint;
            using namespace Stm32Common::Hash;

            const auto argv1 = FNV1a::hash(argv[1]);

#if ENABLE_FP==1

            switch (argv1) {
                case FNV1a::hash("info"): {
                    out()->println("Fingerprint info");
                    out()->printf("fp_nSTDBY : %s\r\n", fp_nSTDBY.isOn() ? "ON" : "OFF");
                    out()->printf("fp_DETECT : %s\r\n", fp_DETECT.isOn() ? "ON" : "OFF");

                    return runReturn::FINISHED;
                }

                case FNV1a::hash("on"): {
                    fp_nSTDBY.setOn();
                    return runReturn::FINISHED;
                }

                case FNV1a::hash("off"): {
                    fp_nSTDBY.setOff();
                    return runReturn::FINISHED;
                }

                case FNV1a::hash("start"): {
                    fpSensor.enqueueEvent(HandShakeEvent{});
                    return runReturn::FINISHED;
                }

                default: { ; }
            }


            if (argv1 == FNV1a::hash("getimage")) {
                const auto r = fpSensor.getImage();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("genchar")) {
                if (argc != 3) return runReturn::ERROR;
                const auto bufferId = static_cast<BufferId>(std::strtoul(argv[2], nullptr, DEC));
                const auto r = fpSensor.genChar(bufferId);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("match")) {
                const auto r = fpSensor.match();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf("matchScore : %d\r\n", r.value().matchScore);
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("search")) {
                if (argc != 5) return runReturn::ERROR;
                const auto bufferId = static_cast<BufferId>(std::strtoul(argv[2], nullptr, DEC));
                const auto startPage = static_cast<uint16_t>(std::strtoul(argv[2], nullptr, DEC));
                const auto countPage = static_cast<uint16_t>(std::strtoul(argv[2], nullptr, DEC));
                const auto r = fpSensor.search(bufferId, startPage, countPage);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf("pageId : %d\r\n", r.value().pageId);
                out()->printf("matchScore : %d\r\n", r.value().matchScore);
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("regmodel")) {
                const auto r = fpSensor.regModel();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("storechar")) {
                if (argc != 4) return runReturn::ERROR;
                const auto bufferId = static_cast<BufferId>(std::strtoul(argv[2], nullptr, DEC));
                const auto pageId = static_cast<PageId>(std::strtoul(argv[2], nullptr, DEC));
                const auto r = fpSensor.storeChar(bufferId, pageId);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("loadchar")) {
                if (argc != 4) return runReturn::ERROR;
                const auto bufferId = static_cast<BufferId>(std::strtoul(argv[2], nullptr, DEC));
                const auto pageId = static_cast<PageId>(std::strtoul(argv[2], nullptr, DEC));
                const auto r = fpSensor.loadChar(bufferId, pageId);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("upchar")) {
                if (argc != 3) return runReturn::ERROR;
                const auto bufferId = static_cast<BufferId>(std::strtoul(argv[2], nullptr, DEC));
                PsUpCharEvent::template_t tpl{sizeof(data), data};
                const auto r = fpSensor.upChar(bufferId, tpl);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("downchar")) {
                PsUpCharEvent::template_t tpl{sizeof(data), data};
                const auto r = fpSensor.downChar(1, tpl);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("upimage")) {
                PsUpImageEvent::image_t image{sizeof(data), data};
                const auto r = fpSensor.upImage(image);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("downimage")) {
                PsDownImageEvent::image_t image{sizeof(data), data};
                const auto r = fpSensor.downImage(image);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("deletechar")) {
                if (argc != 4) return runReturn::ERROR;
                const auto pageId = static_cast<PageId>(std::strtoul(argv[2], nullptr, DEC));
                const auto n = static_cast<uint8_t>(std::strtoul(argv[3], nullptr, DEC));
                const auto r = fpSensor.deleteChar(pageId, n);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("empty")) {
                const auto r = fpSensor.empty();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("writereg")) {
                if (argc != 4) return runReturn::ERROR;
                const auto regNum = static_cast<uint8_t>(std::strtoul(argv[2], nullptr, DEC));
                const auto reg = static_cast<SensorHiLinkZw0608::register_t>(regNum);
                const auto content = static_cast<uint8_t>(std::strtoul(argv[3], nullptr, DEC));
                const auto r = fpSensor.writeReg(reg, content);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("readsyspara")) {
                PsReadSysParaEvent::sysPara_t sysPara{};
                const auto r = fpSensor.readSysPara(sysPara);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" number            : %d\r\n", sysPara.number);
                out()->printf(" template size     : %d\r\n", sysPara.templateSize);
                out()->printf(" database capacity : %d\r\n", sysPara.databaseCapacity);
                out()->printf(" score level       : %d\r\n", sysPara.scoreLevelCode);
                out()->printf(" device address    : %08x\r\n", sysPara.deviceAddress);
                out()->printf(" packet size       : %d\r\n", sysPara.packetSize);
                out()->printf(" baud rate         : %d\r\n", sysPara.baudRate);
                return runReturn::FINISHED;
            }


            if (argv1 == FNV1a::hash("getrandomcode")) {
                const auto r = fpSensor.getRandomCode();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" random number     : %d\r\n", r.value().randomNumber);
                return runReturn::FINISHED;
            }


            if (argv1 == FNV1a::hash("readinfpage") || argv1 == FNV1a::hash("inf")) {
                const auto r = fpSensor.readInfPage();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" enroll times      : %d\r\n", r.value().enrollTimes);
                out()->printf(" template size     : %d\r\n", r.value().templateSize);
                out()->printf(" database capacity : %d\r\n", r.value().databaseCapacity);
                out()->printf(" score level       : %d\r\n", r.value().scoreLevel);
                out()->printf(" device address    : %08x\r\n", r.value().deviceAddress);
                out()->printf(" packet size       : %d\r\n", r.value().packetSize);
                out()->printf(" baud rate         : %d\r\n", r.value().baudRate * 9600);
                out()->printf(" Anti-fake fingerp : %d\r\n", r.value().antiFakeFingerPrint);
                out()->printf(" sensor parameters : %d\r\n", r.value().sensorParameters);
                out()->printf(" encryption level  : %d\r\n", r.value().encryptionLevel);
                out()->printf(" enroll logic      : %d\r\n", r.value().enrollLogic);
                out()->printf(" image format      : %d\r\n", r.value().imageFormat);
                out()->printf(" serial port delay : %d\r\n", r.value().serialPortDelay);
                out()->printf(" Product s/n       : %.*s\r\n", 8, r.value().productSn);
                out()->printf(" Software version  : %.*s\r\n", 8, r.value().softwareVersion);
                out()->printf(" Manufacturer      : %.*s\r\n", 8, r.value().manufacturer);
                out()->printf(" Sensor name       : %.*s\r\n", 8, r.value().sensorName);
                out()->printf(" password          : %08x\r\n", r.value().password);
                out()->printf(" JTAG lock flag    : %08x\r\n", r.value().jtagLockFlag);
                out()->printf(" res               : %04x\r\n", r.value().res1);
                out()->printf(" res               : %04x\r\n", r.value().res2);
                out()->printf(" res               : %04x\r\n", r.value().res3);
                out()->printf(" Valid flag        : %04x\r\n", r.value().validFlag);
                return runReturn::FINISHED;
            }


            if (argv1 == FNV1a::hash("validtemplatenum")) {
                const auto r = fpSensor.validTemplateNum();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" valid template num: %d\r\n", r.value().validTemplateNum);
                return runReturn::FINISHED;
            }


            if (argv1 == FNV1a::hash("getenrollimage")) {
                const auto r = fpSensor.getEnrollImage();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }


            if (argv1 == FNV1a::hash("cancel")) {
                const auto r = fpSensor.cancel();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }


            if (argv1 == FNV1a::hash("autoenroll")) {
                if (argc != 5) return runReturn::ERROR;
                const auto fingerprintId = static_cast<FingerprintId>(std::strtoul(argv[2], nullptr, DEC));
                const auto numberOfEntries = static_cast<uint8_t>(std::strtoul(argv[3], nullptr, DEC));
                const auto parameter = static_cast<AutoEnrollParameter>(std::strtoul(argv[4], nullptr, DEC));
                const auto r = fpSensor.autoEnroll(fingerprintId, numberOfEntries, parameter);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" parameter 1       : 0x%02x\r\n", r.value().parameter1);
                out()->printf(" parameter 2       : 0x%02x\r\n", r.value().parameter2);
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("autoidentify")) {
                if (argc != 5) return runReturn::ERROR;
                const auto scoreLevel = static_cast<ScoreLevel>(std::strtoul(argv[2], nullptr, DEC));
                const auto fingerprintId = static_cast<FingerprintId>(std::strtoul(argv[3], nullptr, DEC));
                const auto parameter = static_cast<AutoIdentifyParameter>(std::strtoul(argv[4], nullptr, DEC));
                const auto r = fpSensor.autoIdentify(scoreLevel, fingerprintId, parameter);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" parameter         : 0x%02x\r\n", r.value().parameter);
                out()->printf(" fingerprintId     : %d\r\n", r.value().fingerprintId);
                out()->printf(" score             : %d\r\n", r.value().score);
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("sleep")) {
                const auto r = fpSensor.sleep();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("getchipsn")) {
                const auto r = fpSensor.getChipSN();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" Chip SN           : %.*s\r\n", 32, r.value().chipSn);
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("handshake")) {
                const auto r = fpSensor.handShake();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("checksensor")) {
                const auto r = fpSensor.checkSensor();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("restsetting")) {
                const auto r = fpSensor.restSetting();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("controlbln")) {
                if (argc != 6) return runReturn::ERROR;
                const auto function = static_cast<ControlBLNFunction>(std::strtoul(argv[2], nullptr, DEC));
                const auto startColor = static_cast<ControlBLNColor>(std::strtoul(argv[3], nullptr, DEC));
                const auto endColor = static_cast<ControlBLNColor>(std::strtoul(argv[4], nullptr, DEC));
                const auto cycles = static_cast<uint8_t>(std::strtoul(argv[4], nullptr, DEC));
                const auto r = fpSensor.controlBLN(function, startColor, endColor, cycles);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("getimageinfo")) {
                const auto r = fpSensor.getImageInfo();
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" image area        : 0x%02x\r\n", r.value().imageArea);
                out()->printf(" image quality     : 0x%02x\r\n", r.value().imageQuality);
                return runReturn::FINISHED;
            }

            if (argv1 == FNV1a::hash("searchnow")) {
                if (argc != 4) return runReturn::ERROR;
                const auto startPage = static_cast<PageId>(std::strtoul(argv[2], nullptr, DEC));
                const auto pageCount = static_cast<uint16_t>(std::strtoul(argv[3], nullptr, DEC));
                const auto r = fpSensor.searchNow(startPage, pageCount);
                if (r.isError()) {
                    out()->printf("ERROR: %s (0x%02x)\r\n", r.error().to_string(), r.error());
                    return runReturn::ERROR;
                }
                out()->printf(" page id           : 0x%02x\r\n", r.value().pageId);
                out()->printf(" match score       : 0x%02x\r\n", r.value().matchScore);
                return runReturn::FINISHED;
            }


#endif

            return runReturn::ERROR;
        }
    };
}
