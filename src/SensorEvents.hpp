/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "FSM/EventInterface.hpp"

using namespace AppCore::FSM;

namespace Stm32Fingerprint {
    namespace Events {
        struct LoopEvent : EventInterface, QueueableEvent {
            LoopEvent() : EventInterface("LoopEvent") { ; }
        };

        struct InitializeEvent : EventInterface, QueueableEvent {
            InitializeEvent() : EventInterface("InitializeEvent") { ; }
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
