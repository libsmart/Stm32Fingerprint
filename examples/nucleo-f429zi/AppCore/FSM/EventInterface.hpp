/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef AT_FIRMWARE_APPCORE_FSM_EVENTINTERFACE_HPP
#define AT_FIRMWARE_APPCORE_FSM_EVENTINTERFACE_HPP

#include "Nameable.hpp"

namespace AppCore::FSM {
    class QueueableEvent {
    public:
        virtual ~QueueableEvent() = default;

        virtual void setData(const uint8_t *data) { ; }
    };
    class EventInterface : public Stm32Common::Nameable {
    public:
        EventInterface() = delete;

        explicit EventInterface(const char *name)
            : Nameable(name) {
        }
    };
}
#endif
