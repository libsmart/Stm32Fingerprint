/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsUpImageState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status PsUpImageState::onEnter(const PsUpImageEvent &event) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    image = event.image;
    imagesize = 0;
    offset = 0;

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_UpImage, nullptr, 0);

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsUpImageState::handle(const DataReceivedEvent &event) {
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
                    ->printf("PS_UpImage(): OK\r\n");
        } else {
            log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_UpImage(): ERROR\r\n");
        }
    }

    if (packageId == 0x02 || packageId == 0x08) {
        // Data package
        // const uint16_t packetLength = getMachine()->rxFrame[7] | (getMachine()->rxFrame[8] << 8);
        const size_t packetLength = getMachine()->be16(&getMachine()->rxFrame[7]);
        const size_t dataLength = packetLength - 2;
        const size_t copySize = std::min(dataLength, image->size - offset);

        log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
                ->printf("packetLength = %d | copySize = %d\r\n", packetLength, copySize);

        memcpy(image->data + offset, &getMachine()->rxFrame[9], copySize);
        offset += copySize;
        imagesize += dataLength;
    }

    if (packageId == 0x08) {
        // Last data packet

        image->size = imagesize;

        log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
                ->printf("offset = %d | imagesize = %d\r\n", offset, imagesize);

        return TransitionTo<ReadyState>{};
    }

    return DoNothing{};
}

DoNothing PsUpImageState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

void PsUpImageState::restartTimeout() {
    stateEnteredMillis = millis();
}

