/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once
#include <stdexcept>

namespace AppCore::FSM {
    class EventQueueEmptyException final : public std::runtime_error {
    public:
        EventQueueEmptyException() : runtime_error("EventQueueEmptyException") { ; }

        explicit EventQueueEmptyException(const std::string &__arg) : runtime_error(__arg) { ; }

        explicit EventQueueEmptyException(const char *string) : runtime_error(string) { ; }
    };
}
