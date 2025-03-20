/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "FSM/EventInterface.hpp"
#include "String/FixedString.hpp"

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
            CommandEvent(): CommandEvent(0, nullptr, 0) { ; }

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

        struct GetChipSnEvent : EventInterface, QueueableEvent {
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
