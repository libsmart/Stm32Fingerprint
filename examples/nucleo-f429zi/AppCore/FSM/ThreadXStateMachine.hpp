/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <stdexcept>
#include "StateMachine.hpp"
#include "Process/ProcessInterface.hpp"
#include "Queue/Queue.hpp"

namespace AppCore::FSM {
    template<typename FsmEvents_t, typename... FsmStates_t>
    class ThreadXStateMachine : public StateMachine<FsmStates_t...>,
                                public Stm32Common::Process::ProcessInterface,
                                public Stm32Common::Nameable,
                                public Stm32ItmLogger::Loggable {
    public:
        explicit ThreadXStateMachine(FsmStates_t... states) : StateMachine<FsmStates_t...>(std::move(states)...) { ; }

        ThreadXStateMachine(FsmStates_t... states, const char *name, Stm32ItmLogger::LoggerInterface *logger)
            : StateMachine<FsmStates_t...>(std::move(states)...),
              Nameable(name),
              Loggable(logger) { ; }

        /**
         * Sets up the necessary configurations or initial conditions required for the FSM.
         * @note Must be called AFTER ThreadX is initialized.
         */
        void setup() override {
            queue.create(LIBSMART_CEIL_DIV(sizeof(EncodedEvent), sizeof(ULONG)));
        }

        /**
         * Continuously executes the state machine processing loop.
         * Extracts the next event from the event queue, processes it through the state machine,
         * and ensures proper error handling in case of exceptions.
         *
         * @note This function operates as the main execution loop for the state machine and should
         *       be invoked regularly to ensure timely event handling and state transitions.
         *
         * @warning Exceptions are caught to prevent disruption of the process flow,
         *          but it's assumed that severe exceptions are handled elsewhere.
         */
        void loop() override {
            try {
                if (queue.isEmpty()) return;
                auto event = dequeueEvent();
                std::visit([this](auto &&actualEvent) {
                    handle(actualEvent);
                }, event);
            } catch (const std::exception &e) {
                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                        ->printf("ERROR: %s\r\n", e.what());
            }
        }

        void end() override {
            queue.flush();
            queue.del();
        }

        void errorHandler() override {
            queue.flush();
            queue.del();
        }


        using EncodedEvent = struct {
            uint8_t eventId;
            uint8_t eventData[sizeof(FsmEvents_t)];
            uint8_t padding[(sizeof(FsmEvents_t) + 1) % sizeof(unsigned long) == 0
                                ? 0
                                : sizeof(unsigned long) - (sizeof(FsmEvents_t) + 1) % sizeof(unsigned long)];
        };


        /**
         * Handles an incoming event and processes it through the state machine.
         * Dispatches the event to the appropriate state in the state machine for handling.
         *
         * @param event The event object to be handled by the state machine.
         * @return Status indicating the result of the event handling, which could include
         *         continuation, termination, or reissuing the event.
         *
         * @attention You are using ThreadXStateMachine. Most likely you do not want to use this function because it
         * executes in the caller's thread context. Use enqueueEvent() instead to add the event to the queue.
         */
        template<typename Event>
        auto handle(const Event &event) -> Status {
            return StateMachine<FsmStates_t...>::handle(event);
        }

        bool enqueueEvent(const FsmEvents_t &event) {
            std::visit([this](auto &&arg) {
                log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                        ->printf("%s::enqueueEvent(%s)\r\n", getName(), arg.getName());
            }, event);

            EncodedEvent encoded_event{};
            encoded_event.eventId = event.index();

            std::visit([&encoded_event](auto &&arg) {
                std::memcpy(encoded_event.eventData, &arg, std::min(sizeof(arg), sizeof(encoded_event.eventData)));
            }, event);

            return queue.send(&encoded_event, TX_NO_WAIT) == TX_SUCCESS;
        }

        // Hilfsstruktur für die Deserialisierung
        template<std::size_t Index, typename VariantType>
        struct VariantDeserializer {
            static FsmEvents_t deserialize(const EncodedEvent &encodedEvent) {
                if (Index == encodedEvent.eventId) {
                    using CurrentType = std::variant_alternative_t<Index, VariantType>;
                    auto variant = CurrentType();
                    variant.setData(encodedEvent.eventData);
                    return variant;
                }
                return VariantDeserializer<Index - 1, VariantType>::deserialize(encodedEvent);
            }
        };

        // Spezialisierung zum Beenden der Rekursion
        template<typename VariantType>
        struct VariantDeserializer<0, VariantType> {
            static FsmEvents_t deserialize(const EncodedEvent &encodedEvent) {
                if (0 == encodedEvent.eventId) {
                    using CurrentType = std::variant_alternative_t<0, VariantType>;
                    auto variant = CurrentType();
                    variant.setData(encodedEvent.eventData);
                    return variant;
                }
                throw std::invalid_argument("Ungültiger eventId");
            }
        };

        FsmEvents_t deserializeEvent(const EncodedEvent &encodedEvent) {
            constexpr std::size_t variantSize = std::variant_size_v<FsmEvents_t>;
            return VariantDeserializer<variantSize - 1, FsmEvents_t>::deserialize(encodedEvent);
        }

        FsmEvents_t dequeueEvent() {
            // log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            // ->printf("ThreadXStateMachine::dequeueEvent()\r\n");

            EncodedEvent encoded_event{};
            auto ret = queue.receive(&encoded_event, TX_NO_WAIT);
            if (ret == TX_SUCCESS) {
                auto event = deserializeEvent(encoded_event);
                return event;
            }

            throw std::runtime_error("Queue empty");
        }

    protected:
        Stm32ThreadX::Queue queue{
            (std::string(getName()) + std::string("::ThreadXStateMachine::Queue")).c_str(), queueMem, sizeof(queueMem),
            getLogger()
        };

    private:
        uint8_t queueMem[512]{};
    };
}
