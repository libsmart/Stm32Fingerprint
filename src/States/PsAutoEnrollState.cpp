/*
 * SPDX-FileCopyrightText: 2026 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PsAutoEnrollState.hpp"
#include "SensorHiLinkZw0608.hpp"

using namespace Stm32Fingerprint::Events;
using namespace Stm32Fingerprint::States;
using Severity = Stm32ItmLogger::LoggerInterface::Severity;

Status PsAutoEnrollState::onEnter(const PsAutoEnrollEvent &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::onEnter(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    result = event.result;

    const struct [[gnu::packed]] data_t {
        FingerprintId fingerprintId;
        uint8_t numberOfEntries;
        uint16_t parameter;
    } data = {
        getMachine()->swapEndian(event.fingerprintId),
        getMachine()->swapEndian(event.numberOfEntries),
        getMachine()->swapEndian(static_cast<uint16_t>(event.parameter))
    };

    getMachine()->sendCommand(SensorHiLinkZw0608::Command::PS_AutoEnroll, (uint8_t *) &data, sizeof(data));

    return Continue{};
}

OneOf<DoNothing, TransitionTo<ReadyState> > PsAutoEnrollState::handle(const DataReceivedEvent &event) {
    log(Severity::DEBUGGING)->printf("%s::%s::handle(%s)\r\n", getMachine()->getName(), getName(), event.getName());

    restartTimeout();

    const auto packageId = getMachine()->rxData.packageId;
    const auto packetLength = getMachine()->rxData.packetLength;

    if (packageId == 0x07 && packetLength == 5) {
        // Command response packet
        const auto confirmation = Confirmation::Code{getMachine()->rxFrame[9]};
        if ((getMachine()->lastConfirmationCode == Confirmation::Code::OK) || (
                getMachine()->lastConfirmationCode == Confirmation::Code::UNKNOWN)) {
            getMachine()->lastConfirmationCode = confirmation;
        }
        const auto parameter1 = getMachine()->rxFrame[10];
        const auto parameter2 = getMachine()->rxFrame[11];

        switch (parameter1) {
            case 0x00: {
                // Instruction legality check
                result->instructionLegalityCheckConfirmation = confirmation;
                if (confirmation == Confirmation::Code::OK) {
                    log(Severity::NOTICE)
                            ->printf("PS_AutoEnroll(): Instruction legality check OK\r\n");
                } else {
                    log(Severity::ERROR)
                            ->printf("PS_AutoEnroll(): Instruction legality check ERROR\r\n");
                    return TransitionTo<ReadyState>{};
                }
                break;
            }

            case 0x01: {
                // Picture result
                const auto step = parameter2;
                // result->pictureDrawingResultConfirmation = Confirmation::Code{confirmation};
                if (confirmation == Confirmation::Code::OK) {
                    log(Severity::NOTICE)->printf("PS_AutoEnroll(): #%d Picture result OK\r\n", step);
                } else {
                    log(Severity::ERROR)->printf("PS_AutoEnroll(): #%d Picture result ERROR\r\n", step);
                }
                break;
            }

            case 0x02: {
                // Generate feature result
                const auto step = parameter2;
                // result->pictureDrawingResultConfirmation = Confirmation::Code{confirmation};
                if (confirmation == Confirmation::Code::OK) {
                    log(Severity::NOTICE)->printf("PS_AutoEnroll(): #%d Generate feature result OK\r\n", step);
                } else {
                    log(Severity::ERROR)->printf("PS_AutoEnroll(): #%d Generate feature result ERROR\r\n", step);
                }
                break;
            }

            case 0x03: {
                // Finger away
                const auto step = parameter2;
                // result->pictureDrawingResultConfirmation = Confirmation::Code{confirmation};
                if (confirmation == Confirmation::Code::OK) {
                    log(Severity::NOTICE)->printf("PS_AutoEnroll(): #%d Finger away OK\r\n", step);
                } else {
                    log(Severity::ERROR)->printf("PS_AutoEnroll(): #%d Finger away ERROR\r\n", step);
                }
                break;
            }

            case 0x04: {
                // Merge templates
                // result->pictureDrawingResultConfirmation = Confirmation::Code{confirmation};
                if (confirmation == Confirmation::Code::OK) {
                    log(Severity::NOTICE)->printf("PS_AutoEnroll(): Merge templates OK\r\n");
                } else {
                    log(Severity::ERROR)->printf("PS_AutoEnroll(): Merge templates ERROR\r\n");
                }
                break;
            }

            case 0x05: {
                // Check register
                // result->pictureDrawingResultConfirmation = Confirmation::Code{confirmation};
                if (confirmation == Confirmation::Code::OK) {
                    log(Severity::NOTICE)->printf("PS_AutoEnroll(): Check register OK\r\n");
                } else {
                    log(Severity::ERROR)->printf("PS_AutoEnroll(): Check register ERROR\r\n");
                    return TransitionTo<ReadyState>{};
                }
                break;
            }

            case 0x06: {
                // Template storage
                // result->pictureDrawingResultConfirmation = Confirmation::Code{confirmation};
                if (confirmation == Confirmation::Code::OK) {
                    log(Severity::NOTICE)->printf("PS_AutoEnroll(): Template storage OK\r\n");
                    return TransitionTo<ReadyState>{};
                } else {
                    log(Severity::ERROR)->printf("PS_AutoEnroll(): Template storage ERROR\r\n");
                    return TransitionTo<ReadyState>{};
                }
                break;
            }


            default: { ; }
        }
    }

    return DoNothing{};
}

DoNothing PsAutoEnrollState::handle(const LoopEvent &event) {
    if (millis() - stateEnteredMillis > TIMEOUT_CMD) {
        log(Severity::ERROR)->printf("PS_AutoEnroll(): TIMEOUT\r\n");
        getMachine()->lastConfirmationCode = Confirmation::Code::ERROR_TIMEOUT;
        getMachine()->enqueueEvent(TimeoutEvent{});
    }
    return {};
}

void PsAutoEnrollState::restartTimeout() {
    stateEnteredMillis = millis();
}

