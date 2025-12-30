/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsDownImageState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status PsDownImageState::onEnter(const PsDownImageEvent &event) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    image = event.image;

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_DownImage, nullptr, 0);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsDownImageState::handle(const DataReceivedEvent &event) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    const auto packageId = getMachine()->rxFrame[6];

    if (packageId == 0x07) {
        // Command response packet
        const auto confirmation = getMachine()->rxFrame[9];
        getMachine()->lastConfirmationCode = static_cast<Confirmation::Code>(confirmation);

        if (confirmation == 0x00) {
            log(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                    ->printf("PS_DownImage(): OK\r\n");
        } else {
            log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_DownImage(): ERROR\r\n");
        }
    }

    if (packageId == 0x02 || packageId == 0x08) {
        // Data package
        const uint16_t packetLength = getMachine()->rxFrame[7] | (getMachine()->rxFrame[8] << 8);

    }

    if (packageId == 0x08) {
        // Last data packet
        return TransitionTo<ReadyState>{};
    }

    return DoNothing{};
}

DoNothing PsDownImageState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

void PsDownImageState::restartTimeout() {
    stateEnteredMillis = millis();
}

