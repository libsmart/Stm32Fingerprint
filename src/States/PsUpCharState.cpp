/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsUpCharState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status PsUpCharState::onEnter(const PsUpCharEvent &event) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    tpl = event.tpl;
    bufferId = event.bufferId;
    const struct [[gnu::packed]] data_t {
        BufferId bufferId;
    } data = {getMachine()->swapEndian(bufferId)};

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_UpChar, (uint8_t *)&data, sizeof(data));

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsUpCharState::handle(const DataReceivedEvent &event) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    const auto packageId = getMachine()->rxData.packageId;

    if (packageId == 0x07) {
        // Command response packet
        const auto confirmation = getMachine()->rxFrame[9];
        getMachine()->lastConfirmationCode = Confirmation::Code{confirmation};

        if (confirmation == 0x00) {
            log(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                    ->printf("PS_UpChar(): OK\r\n");
        } else {
            log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                    ->printf("PS_UpChar(): ERROR\r\n");
            return TransitionTo<ReadyState>{};
        }
    }

    if (packageId == 0x02 || packageId == 0x08) {
        // Data package
        // const uint16_t packetLength = getMachine()->rxFrame[7] | (getMachine()->rxFrame[8] << 8);
        const size_t packetLength = getMachine()->rxData.packetLength;
        const size_t dataLength = packetLength - 2;
        const size_t copySize = std::min(dataLength, tpl->size - bufferOffset);

        log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
                ->printf("packetLength = %d | copySize = %d\r\n", packetLength, copySize);

        memcpy(tpl->data + bufferOffset, &getMachine()->rxFrame[9], copySize);
        bufferOffset += copySize;
        downloadSize += dataLength;
    }

    if (packageId == 0x08) {
        // Last data packet

        log(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)
                ->printf("offset = %d | imagesize = %d\r\n", bufferOffset, downloadSize);

        return TransitionTo<ReadyState>{};
    }

    return DoNothing{};
}

DoNothing PsUpCharState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

void PsUpCharState::restartTimeout() {
    stateEnteredMillis = millis();
}

