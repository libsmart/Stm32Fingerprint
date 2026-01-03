/*
 * SPDX-FileCopyrightText: 2026 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsAutoIdentifyState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;

Status PsAutoIdentifyState::onEnter(const PsAutoIdentifyEvent &event) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    result = event.result;

    const struct [[gnu::packed]] data_t {
        ScoreLevel scoreLevel;
        FingerprintId fingerprintId;
        uint16_t parameter;
    } data = {
        getMachine()->swapEndian(event.scoreLevel),
        getMachine()->swapEndian(event.fingerprintId),
        getMachine()->swapEndian((uint16_t) event.parameter)
    };

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_AutoIdentify, (uint8_t *) &data, sizeof(data));

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsAutoIdentifyState::handle(const DataReceivedEvent &event) {
    log(Stm32ItmLogger::LoggerInterface::Severity::DEBUGGING)
            ->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    const auto packageId = getMachine()->rxData.packageId;
    const auto packetLength = getMachine()->rxData.packetLength;

    if (packageId == 0x07 && packetLength == 8) {
        // Command response packet
        const auto confirmation = getMachine()->rxFrame[9];
        if ((getMachine()->lastConfirmationCode == Confirmation::Code::OK) || (
                getMachine()->lastConfirmationCode == Confirmation::Code::UNKNOWN)) {
            getMachine()->lastConfirmationCode = Confirmation::Code{confirmation};
        }
        const auto parameter = getMachine()->rxFrame[10];
        const auto idNumber = static_cast<uint16_t>(getMachine()->rxFrame[11] << 8 | getMachine()->rxFrame[12]);
        const auto score = static_cast<uint16_t>(getMachine()->rxFrame[13] << 8 | getMachine()->rxFrame[14]);

        switch (parameter) {
            case 0x00: {
                // Instruction legality check
                result->instructionLegalityCheckConfirmation = Confirmation::Code{confirmation};
                if (confirmation == 0x00) {
                    log(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                            ->printf("PS_UpChar(): Instruction legality check OK\r\n");
                } else {
                    log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                            ->printf("PS_UpChar(): Instruction legality check ERROR\r\n");
                }
                break;
            }

            case 0x01: {
                // Picture drawing result
                result->pictureDrawingResultConfirmation = Confirmation::Code{confirmation};
                if (confirmation == 0x00) {
                    log(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                            ->printf("PS_UpChar(): Picture drawing result OK\r\n");
                } else {
                    log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                            ->printf("PS_UpChar(): Picture drawing result ERROR\r\n");
                }
                break;
            }

            case 0x05: {
                // search results
                result->searchResultConfirmation = Confirmation::Code{confirmation};
                result->fingerprintId = idNumber;
                result->score = Score{score};
                if (confirmation == 0x00) {
                    log(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                            ->printf("PS_UpChar(): search results OK\r\n");
                } else {
                    log(Stm32ItmLogger::LoggerInterface::Severity::ERROR)
                            ->printf("PS_UpChar(): search results ERROR\r\n");
                }
                break;
            }

            default: { ; }
        }
    }

    return DoNothing{};
}

DoNothing PsAutoIdentifyState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

void PsAutoIdentifyState::restartTimeout() {
    stateEnteredMillis = millis();
}

