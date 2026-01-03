/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <optional>

#include "FSM/EventInterface.hpp"
#include "String/FixedString.hpp"
#include "Types/Types.hpp"

using namespace AppCore::FSM;

namespace Stm32Fingerprint {
    namespace Events {
        struct LoopEvent : EventInterface, QueueableEvent {
            LoopEvent() : EventInterface("LoopEvent") { ; }
        };

        struct InitializeEvent : EventInterface, QueueableEvent {
            InitializeEvent() : EventInterface("InitializeEvent") { ; }
        };

        struct CommandEvent : EventInterface, QueueableEvent {
            CommandEvent() : CommandEvent(0, nullptr, 0) { ; }

            explicit CommandEvent(const uint8_t command) : CommandEvent(command, nullptr, 0) { ; }

            template<std::size_t N>
            CommandEvent(const uint8_t command, const Stm32Common::String::FixedString<N> fixedString)
                : CommandEvent(command, fixedString.c_str(), fixedString.size()) { ; }

            CommandEvent(const uint8_t command, const uint8_t *data, const uint16_t data_length)
                : EventInterface("CommandEvent"), command(command), dataLength(data_length) {
                if (data != nullptr) {
                    memcpy(const_cast<uint8_t *>(this->data), data,
                           std::min(static_cast<size_t>(data_length), sizeof(data)));
                }
            }

            uint8_t command;
            const uint8_t data[8]{};
            uint16_t dataLength;

            void setData(const uint8_t *data) override {
                std::remove_reference_t<decltype(*this)> me;
                memcpy(&me, data, sizeof(me));
                command = me.command;
                dataLength = me.dataLength;
                memcpy(const_cast<uint8_t *>(this->data), me.data, sizeof(data));
            }
        };

        struct PsUpCharEvent final : EventInterface, QueueableEvent {
            using template_t = struct template_t {
                size_t size;
                uint8_t *data;
            };

            PsUpCharEvent() : PsUpCharEvent(1, nullptr) { ; }

            explicit PsUpCharEvent(const uint8_t bufferId, template_t *tpl)
                : EventInterface("PsUpCharEvent"), bufferId(bufferId) { PsUpCharEvent::tpl = tpl; }

            BufferId bufferId;
            template_t *tpl{};

            void setData(const uint8_t *data) override {
                std::remove_reference_t<decltype(*this)> me;
                memcpy(&me, data, sizeof(me));
                tpl = me.tpl;
                bufferId = me.bufferId;
            }
        };

        struct PsUpImageEvent final : EventInterface, QueueableEvent {
            using image_t = struct image_t {
                size_t size;
                uint8_t *data;
            };

            PsUpImageEvent() : PsUpImageEvent(nullptr) { ; }

            explicit PsUpImageEvent(image_t *image)
                : EventInterface("PsUpImageEvent") { PsUpImageEvent::image = image; }

            image_t *image{};

            void setData(const uint8_t *data) override {
                std::remove_reference_t<decltype(*this)> me;
                memcpy(&me, data, sizeof(me));
                image = me.image;
            }
        };

        struct PsDownImageEvent final : EventInterface, QueueableEvent {
            using image_t = struct image_t {
                size_t size;
                uint8_t *data;
            };

            PsDownImageEvent() : PsDownImageEvent(nullptr) { ; }

            explicit PsDownImageEvent(image_t *image)
                : EventInterface("PsDownImageEvent") { PsDownImageEvent::image = image; }

            image_t *image{};

            void setData(const uint8_t *data) override {
                std::remove_reference_t<decltype(*this)> me;
                memcpy(&me, data, sizeof(me));
                image = me.image;
            }
        };

        struct PsReadSysParaEvent final : EventInterface, QueueableEvent {
            using sysPara_t = struct sysPara_t {
                uint16_t number;
                uint16_t templateSize;
                uint16_t databaseCapacity;
                uint16_t scoreLevelCode;
                uint32_t deviceAddress;
                uint16_t packetSize;
                uint16_t baudRate;
            };

            PsReadSysParaEvent() : PsReadSysParaEvent(nullptr) { ; }

            explicit PsReadSysParaEvent(sysPara_t *sysPara)
                : EventInterface("PsReadSysParaEvent") { PsReadSysParaEvent::sysPara = sysPara; }

            sysPara_t *sysPara{};

            void setData(const uint8_t *data) override {
                std::remove_reference_t<decltype(*this)> me;
                memcpy(&me, data, sizeof(me));
                sysPara = me.sysPara;
            }
        };

        struct PsReadInfPageEvent final : EventInterface, QueueableEvent {
            PsReadInfPageEvent() : EventInterface("PsReadInfPageEvent") { ; }
        };

        struct PsAutoIdentifyEvent final : EventInterface, QueueableEvent {
            PsAutoIdentifyEvent() : PsAutoIdentifyEvent(0, 0, AutoIdentifyParameter{0}, nullptr) { ; }

            PsAutoIdentifyEvent(const ScoreLevel score_level, const FingerprintId fingerprint_id,
                                const AutoIdentifyParameter parameter, const AutoIdentifyResult *result)
                : EventInterface("PsAutoIdentifyEvent"),
                  scoreLevel(score_level), fingerprintId(fingerprint_id), parameter(parameter), result(result) { ; }

            ScoreLevel scoreLevel;
            FingerprintId fingerprintId;
            AutoIdentifyParameter parameter;
            const AutoIdentifyResult *result;

            void setData(const uint8_t *data) override {
                std::remove_reference_t<decltype(*this)> me;
                memcpy(&me, data, sizeof(me));
                scoreLevel = me.scoreLevel;
                fingerprintId = me.fingerprintId;
                parameter = me.parameter;
                result = me.result;
            }
        };

        struct GetChipSnEvent final : EventInterface, QueueableEvent {
            GetChipSnEvent() : EventInterface("GetChipSnEvent") { ; }
        };

        struct HandShakeEvent : EventInterface, QueueableEvent {
            HandShakeEvent() : EventInterface("HandShakeEvent") { ; }
        };

        struct WakeupEvent : EventInterface, QueueableEvent {
            WakeupEvent() : EventInterface("WakeupEvent") { ; }
        };

        struct InitOkReceivedEvent : EventInterface, QueueableEvent {
            InitOkReceivedEvent() : EventInterface("InitOkReceivedEvent") { ; }
        };

        struct DataReceivedEvent : EventInterface, QueueableEvent {
            DataReceivedEvent() : EventInterface("DataReceivedEvent") { ; }
        };

        struct DetectEvent : EventInterface, QueueableEvent {
            DetectEvent() : EventInterface("DetectEvent") { ; }
        };

        struct TimeoutEvent : EventInterface, QueueableEvent {
            TimeoutEvent() : EventInterface("TimeoutEvent") { ; }
        };

        struct ResetEvent : EventInterface, QueueableEvent {
            ResetEvent() : EventInterface("ResetEvent") { ; }
        };

        struct ErrorEvent : EventInterface, QueueableEvent {
            ErrorEvent() : EventInterface("ErrorEvent") { ; }
        };
    }

    using Events_t = std::variant<
        Events::LoopEvent,
        Events::InitializeEvent,
        Events::CommandEvent,
        Events::PsUpCharEvent,
        Events::PsUpImageEvent,
        Events::PsDownImageEvent,
        Events::PsReadSysParaEvent,
        Events::PsReadInfPageEvent,
        Events::PsAutoIdentifyEvent,
        Events::GetChipSnEvent,
        Events::HandShakeEvent,
        Events::WakeupEvent,
        Events::InitOkReceivedEvent,
        Events::DataReceivedEvent,
        Events::DetectEvent,
        Events::TimeoutEvent,
        Events::ResetEvent,
        Events::ErrorEvent
    >;
}
