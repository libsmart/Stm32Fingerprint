/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef AT_FIRMWARE_APPCORE_FSM_STATEINTERFACE_HPP
#define AT_FIRMWARE_APPCORE_FSM_STATEINTERFACE_HPP

#include "Loggable.hpp"
#include "Nameable.hpp"
#include "StateMachine.hpp"

namespace AppCore::FSM {
    template<class Machine>
    class StateInterface
            : public Stm32Common::Nameable,
              public Stm32ItmLogger::Loggable {
    public:
        StateInterface() = delete;

        explicit StateInterface(const char *name, Machine *machine)
            : StateInterface(name, machine, &Stm32ItmLogger::emptyLogger) { ; }

        StateInterface(const char *name, Machine *machine, Stm32ItmLogger::LoggerInterface *logger)
            : Nameable(name),
              Loggable(logger),
              machine(machine) {
        }

        [[nodiscard]] auto *getMachine() const { return machine; }
        [[nodiscard]] auto &fsm() const { return *machine; }

        template<typename... Args>
        Status onEnter(Args... args) {
            log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("%s::%s::onEnter()\r\n", getMachine()->getName(), getName());
            return {};
        }

        template<typename Event>
        Status onEnter(const Event &event) {
            log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());
            return {};
        }

        template<typename... Args>
        Status onLeave(Args... args) {
            log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("%s::%s::onLeave()\r\n", getMachine()->getName(), getName());
            return {};
        }

        template<typename Event>
        Status onLeave(const Event &event) {
            log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
                    ->printf("%s::%s::onLeave(%s)\r\n", getMachine()->getName(), getName(), event.getName());
            return {};
        }

    private:
        Machine *machine;
    };
}
#endif
