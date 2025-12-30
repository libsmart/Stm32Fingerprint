/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "SensorHiLinkZw0608.hpp"
#include "StreamSession/GeneralStreamSession.hpp"

extern Stm32Common::StreamSession::Manager<Stm32Common::StreamSession::GeneralStreamSession, 1> fpSessionManager;

using namespace Stm32Fingerprint;

void SensorHiLinkZw0608::setup() {
    pinDetect.setup();
    pinEnable.setup();
    // pinEnable.setOn();
    flags.create();
    serial.setup();
    ThreadXStateMachine::setup();
}

void SensorHiLinkZw0608::initialize() {
    enqueueEvent(InitializeEvent{});
}

void SensorHiLinkZw0608::loop() {
    pinDetect.loop();
    pinEnable.loop();
    serial.loop();
    ThreadXStateMachine::loop();
    parseReply();
    handle(LoopEvent{});
}

void SensorHiLinkZw0608::end() {
    ThreadXStateMachine::end();
    pinEnable.setOff();
}

void SensorHiLinkZw0608::errorHandler() {
    // ThreadXStateMachine::errorHandler();
    pinEnable.setOff();
}

void SensorHiLinkZw0608::isrDetect() {
    enqueueEvent(DetectEvent{});
}

uint16_t SensorHiLinkZw0608::calc_checksum(uint8_t *data, int32_t start, int32_t end) {
    uint16_t chksum = 0;

    for (int32_t i = start; i < end; ++i) {
        chksum += (uint16_t) data[i];
    }

    return chksum;
}

bool SensorHiLinkZw0608::check_checksum(uint8_t *data, uint32_t len) {
    uint16_t chksum_to_check = 0;
    uint16_t chksum = 0;

    chksum_to_check = data[len - 1] + (data[len - 2] << 8);

    for (int32_t i = 6; i < len - 2; ++i)
        chksum += data[i];

    return (chksum_to_check == chksum);
}

void SensorHiLinkZw0608::parseReply() {
    const auto available = serial.available();

    if (available == 0) return;


    // auto rxBuffer = serial.getRxBuffer();
    const auto rxBuffer = fpSessionManager.getFirstSession()->getRxBuffer();
    const volatile auto buf = rxBuffer->getReadPointer();

    /*
    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)->print("FP: ");
    for (size_t i = 0; i < available; i++) {
        log()->printf("%02x ", buf[i]);
    }
    log()->println();
    */

    size_t frame_length = 0;

    while (rxBuffer->available() > 0) {
        const char c = rxBuffer->read();
        rxFrame[frameBytesParsed++] = c;
        if (parserState != parserState_t::CHECKSUM) rollingChecksum += static_cast<uint16_t>(c);
        switch (parserState) {
            case parserState_t::NONE: {
                parserState = parserState_t::NONE;

                if (c == 0x55) {
                    // fp sensor sends 0x55
                    handle(InitOkReceivedEvent{});
                    frameBytesParsed = 0;
                    memset(rxFrame, 0, sizeof(rxFrame));
                    break;
                }

                if (c == 0xef) {
                    parserState = parserState_t::HEADER;
                    memset(&rxData, 0, sizeof(rxData));
                    rxData.header = c << 8;
                    cnt = 0;
                    break;
                }
            }

            case parserState_t::HEADER: {
                parserState = parserState_t::HEADER;

                if (c == 0x01) {
                    parserState = parserState_t::ADDRESS;
                    rxData.header |= c;
                    cnt = 0;
                }
                break;
            }

            case parserState_t::ADDRESS: {
                parserState = parserState_t::ADDRESS;

                rxData.address = (rxData.address << 8) | c;
                cnt++;
                if (cnt >= ADDRESS_SIZE) {
                    parserState = parserState_t::PACKAGE_ID;
                    cnt = 0;
                    rollingChecksum = 0;
                }
                break;
            }

            case parserState_t::PACKAGE_ID: {
                parserState = parserState_t::PACKAGE_ID;

                rxData.packageId = c;
                parserState = parserState_t::PACKET_LENGTH;
                cnt = 0;
                break;
            }

            case parserState_t::PACKET_LENGTH: {
                parserState = parserState_t::PACKET_LENGTH;

                rxData.packetLength = (rxData.packetLength << 8) | c;
                cnt++;

                if (cnt >= LENGTH_SIZE) {
                    frame_length = rxData.packetLength + DATA_OFFSET; //TODO: Check frame length
                    parserState = parserState_t::DATA;
                    cnt = 0;
                }
                break;
            }

            case parserState_t::DATA: {
                parserState = parserState_t::DATA;

                rxData.data = &rxFrame[DATA_OFFSET];
                cnt++;

                if (cnt >= rxData.packetLength - CHECKSUM_SIZE) {
                    parserState = parserState_t::CHECKSUM;
                    cnt = 0;
                }
                break;
            }

            case parserState_t::CHECKSUM: {
                parserState = parserState_t::CHECKSUM;

                rxData.checksum = (rxData.checksum << 8) | c;
                cnt++;
                if (cnt < CHECKSUM_SIZE) break;
            }

            case parserState_t::CHECK_CHECKSUM: {
                parserState = parserState_t::CHECK_CHECKSUM;

                if (rxData.checksum != rollingChecksum) {
                    parserState = parserState_t::ERROR;
                    break;
                }
            }

            case parserState_t::DONE: {
                parserState = parserState_t::DONE;

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)->print("RX: ");
                for (size_t i = 0; i < rxData.packetLength + DATA_OFFSET; i++) {
                    log()->printf("%02x ", rxFrame[i]);
                }
                log()->println();

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Header         0x%04x\r\n", rxData.header);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Device address 0x%08x\r\n", rxData.address);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Package ID     0x%02x\r\n", rxData.packageId);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Package length 0x%04x (%d)\r\n", rxData.packetLength, rxData.packetLength);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("confirmation   0x%02x\r\n", rxData.data[0]);

                log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::NOTICE)
                        ->printf("Checksum       0x%04x (%d)\r\n", rxData.checksum, rxData.checksum);


                handle(DataReceivedEvent{});


                cnt = 0;
                frameBytesParsed = 0;
                memset(rxFrame, 0, sizeof(rxFrame));
                parserState = parserState_t::NONE;
                break;
            }

            case parserState_t::ERROR: {
                parserState = parserState_t::ERROR;
            }

            default: break;
        }
    }
}

void SensorHiLinkZw0608::sendCommand(const uint8_t instruction) {
    sendPacket(0x01, &instruction, 1);
}

void SensorHiLinkZw0608::sendCommand(const uint8_t instruction, const uint8_t *data, const uint16_t dataLength) {
    uint8_t tmpData[dataLength + 1];
    tmpData[0] = instruction;
    memcpy(tmpData + 1, data, dataLength);
    sendPacket(0x01, tmpData, dataLength + 1);
}

void SensorHiLinkZw0608::sendPacket(const uint8_t packetId, const uint8_t *data, const uint16_t dataLength) {
    const uint16_t frame_length = dataLength + 2 + 9;
    if (frame_length > sizeof(txFrame)) {
        throw std::runtime_error("txPacket too small");
    }

    auto *header = (uint16_t *) &txFrame[0];
    auto *device_address = (uint32_t *) &txFrame[2];
    auto *package_id = (uint8_t *) &txFrame[6];
    auto *package_length = (uint16_t *) &txFrame[7];
    auto *payload = (uint8_t *) &txFrame[9];
    auto *checksum = (uint16_t *) &txFrame[9 + dataLength];

    *header = __builtin_bswap16(0xef01);
    *device_address = __builtin_bswap32(address);
    *package_id = packetId;
    *package_length = __builtin_bswap16(dataLength + 2);

    if (data != nullptr) {
        memcpy(payload, data, dataLength);
    }

    *checksum = __builtin_bswap16(calc_checksum(reinterpret_cast<uint8_t *>(&txFrame), 6, frame_length - 2));

    log()->setSeverity(Stm32ItmLogger::LoggerInterface::Severity::INFORMATIONAL)->print("TX: ");
    for (size_t i = 0; i < frame_length; i++) {
        log()->printf("%02x ", txFrame[i]);
    }
    log()->println();

    serial.getSession()->write(reinterpret_cast<const uint8_t *>(&txFrame), frame_length);
}

ConfirmationResult SensorHiLinkZw0608::getImage() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_GetImage});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::genChar(BufferId bufferId) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        BufferId bufferId;
    } data = {swapEndian(bufferId)};
    enqueueEvent(CommandEvent{PS_GenChar, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

MatchConfirmationResult SensorHiLinkZw0608::match() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_Match});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK) return MatchConfirmationResult::err(lastConfirmationCode);
    return MatchConfirmationResult::ok({
        swapEndian(static_cast<uint16_t>(rxData.data[1] | (swapEndian(rxData.data[2]) << 8)))
    });
}

SearchConfirmationResult SensorHiLinkZw0608::search(const BufferId bufferId, const uint16_t startPage,
                                                    const uint16_t countPage) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        BufferId bufferId;
        uint16_t startPage;
        uint16_t countPage;
    } data = {swapEndian(bufferId), swapEndian(startPage), swapEndian(countPage)};
    enqueueEvent(CommandEvent{PS_StoreChar, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK) return SearchConfirmationResult::err(lastConfirmationCode);
    return SearchConfirmationResult::ok({
        swapEndian(static_cast<uint16_t>(rxData.data[1] | (swapEndian(rxData.data[2]) << 8))),
        swapEndian(static_cast<uint16_t>(rxData.data[3] | (swapEndian(rxData.data[4]) << 8)))
    });
}

ConfirmationResult SensorHiLinkZw0608::regModel() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_RegModel});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::storeChar(const BufferId bufferId, const PageId pageId) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        BufferId bufferId;
        PageId pageId;
    } data = {swapEndian(bufferId), swapEndian(pageId)};
    enqueueEvent(CommandEvent{PS_StoreChar, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::loadChar(const BufferId bufferId, const PageId pageId) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        BufferId bufferId;
        PageId pageId;
    } data = {swapEndian(bufferId), swapEndian(pageId)};
    enqueueEvent(CommandEvent{PS_LoadChar, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::upChar(const BufferId bufferId, PsUpCharEvent::template_t &tpl) {
    clearReadyFlag();
    enqueueEvent(PsUpCharEvent{bufferId, &tpl});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::downChar(const BufferId bufferId, const PsUpCharEvent::template_t &tpl) {
    LIBSMART_UNUSED(bufferId);
    LIBSMART_UNUSED(tpl);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
}

ConfirmationResult SensorHiLinkZw0608::upImage(PsUpImageEvent::image_t &image) {
    clearReadyFlag();
    enqueueEvent(PsUpImageEvent{&image});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::downImage(PsDownImageEvent::image_t &image) {
    LIBSMART_UNUSED(image);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
    // clearReadyFlag();
    // enqueueEvent(PsDownImageEvent{&image});
    // awaitReadyFlag();
    // return lastConfirmationCode;
}

ConfirmationResult SensorHiLinkZw0608::deleteChar(const PageId pageId, const uint16_t count) {
    clearReadyFlag();
    const uint16_t data[2] = {swapEndian(pageId), swapEndian(count)};
    enqueueEvent(CommandEvent{PS_DeleteChar, (uint8_t *) data, sizeof(data)});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::empty() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_Empty});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::writeReg(const register_t reg, const uint8_t content) {
    clearReadyFlag();
    const uint8_t data[2] = {static_cast<uint8_t>(reg), content};
    enqueueEvent(CommandEvent{PS_WriteReg, data, sizeof(data)});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::readSysPara(PsReadSysParaEvent::sysPara_t &sysPara) {
    clearReadyFlag();
    enqueueEvent(PsReadSysParaEvent{&sysPara});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::setPwd(Password password) {
    LIBSMART_UNUSED(password);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
}

ConfirmationResult SensorHiLinkZw0608::vfyPwd(Password password) {
    LIBSMART_UNUSED(password);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
}

GetRandomCodeConfirmationResult SensorHiLinkZw0608::getRandomCode() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_GetRandomCode});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return GetRandomCodeConfirmationResult::err(lastConfirmationCode);
    return GetRandomCodeConfirmationResult::ok({
        swapEndian(
            static_cast<uint32_t>(
                rxData.data[1] | (rxData.data[2] << 8)
                | (rxData.data[3] << 16) | (rxData.data[4] << 24)
            ))
    });
}

ConfirmationResult SensorHiLinkZw0608::setChipAddr(DeviceAddress deviceAddress) {
    LIBSMART_UNUSED(deviceAddress);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
}

ReadInfPageConfirmationResult SensorHiLinkZw0608::readInfPage() {
    clearReadyFlag();
    enqueueEvent(PsReadInfPageEvent{});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return ReadInfPageConfirmationResult::err(lastConfirmationCode);

    constexpr size_t BASE = 9;
    auto ret = ReadInfPageConfirmationResult::ok({
        be16(&rxFrame[BASE + 0]),
        be16(&rxFrame[BASE + 2]),
        be16(&rxFrame[BASE + 4]),
        be16(&rxFrame[BASE + 6]),
        be32(&rxFrame[BASE + 8]),
        be16(&rxFrame[BASE + 12]),
        be16(&rxFrame[BASE + 14]),
        be16(&rxFrame[BASE + 16]),
        be16(&rxFrame[BASE + 18]),
        be16(&rxFrame[BASE + 20]),
        be16(&rxFrame[BASE + 22]),
        be16(&rxFrame[BASE + 24]),
        be16(&rxFrame[BASE + 26]),
        "",
        "",
        "",
        "",
        be32(&rxFrame[BASE + 60]),
        be32(&rxFrame[BASE + 64]),
        be16(&rxFrame[BASE + 68]),
        be16(&rxFrame[BASE + 70]),
        be16(&rxFrame[BASE + 72]),
        be16(&rxFrame[BASE + 134])
    });
    std::memcpy((void *) ret.value().productSn, &rxFrame[BASE + 28], 8);
    std::memcpy((void *) ret.value().softwareVersion, &rxFrame[BASE + 36], 8);
    std::memcpy((void *) ret.value().manufacturer, &rxFrame[BASE + 44], 8);
    std::memcpy((void *) ret.value().sensorName, &rxFrame[BASE + 52], 8);
    return ret;
}

ConfirmationResult SensorHiLinkZw0608::writeNotepad(NotepadPageId notepadPageId, NotepadPageContent content) {
    LIBSMART_UNUSED(notepadPageId);
    LIBSMART_UNUSED(content);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
}

ConfirmationResult SensorHiLinkZw0608::readNotepad(NotepadPageId notepadPageId, NotepadPageContent &content) {
    LIBSMART_UNUSED(notepadPageId);
    LIBSMART_UNUSED(content);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
}

ConfirmationResult SensorHiLinkZw0608::burnCode(UpgradeMode upgradeMode) {
    LIBSMART_UNUSED(upgradeMode);
    return ConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
}

ValidTemplateNumConfirmationResult SensorHiLinkZw0608::validTemplateNum() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_ValidTemplateNum});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return ValidTemplateNumConfirmationResult::err(lastConfirmationCode);
    return ValidTemplateNumConfirmationResult::ok({
        swapEndian(static_cast<uint16_t>(rxData.data[1] | (rxData.data[2] << 8)))
    });
}

ReadIndexTableConfirmationResult SensorHiLinkZw0608::readIndexTable(IndexPageId indexPageId) {
    return ReadIndexTableConfirmationResult::err(Confirmation::Code::ERROR_NOT_IMPLEMENTED);
    // clearReadyFlag();
    // enqueueEvent(CommandEvent{PS_ValidTemplateNum});
    // awaitReadyFlag();
    // if (lastConfirmationCode != Confirmation::Code::OK)
    // return ReadIndexTableConfirmationResult::err(lastConfirmationCode);
    // auto ret = ReadIndexTableConfirmationResult::ok({});
    // constexpr size_t BASE = 9;
    // std::memcpy((void *) ret.value().index, &rxFrame[BASE + 0], 32);
    // return ret;
}

ConfirmationResult SensorHiLinkZw0608::getEnrollImage() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_GetEnrollImage});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::cancel() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_Cancel});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

AutoEnrollConfirmationResult SensorHiLinkZw0608::autoEnroll(const FingerprintId fingerprintId,
                                                            const uint8_t numberOfEntries,
                                                            const AutoEnrollParameter parameter) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        FingerprintId fingerprintId;
        uint8_t numberOfEntries;
        uint16_t parameter;
    } data = {swapEndian(fingerprintId), swapEndian(numberOfEntries), swapEndian((uint16_t) parameter)};
    enqueueEvent(CommandEvent{PS_AutoEnroll, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return AutoEnrollConfirmationResult::err(lastConfirmationCode);
    return AutoEnrollConfirmationResult::ok({
        swapEndian(rxData.data[1]),
        swapEndian(rxData.data[2])
    });
}

AutoIdentifyConfirmationResult SensorHiLinkZw0608::autoIdentify(const ScoreLevel scoreLevel,
                                                                const FingerprintId fingerprintId,
                                                                const AutoIdentifyParameter parameter) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        ScoreLevel scoreLevel;
        FingerprintId fingerprintId;
        uint16_t parameter;
    } data = {swapEndian(scoreLevel), swapEndian(fingerprintId), swapEndian((uint16_t) parameter)};
    enqueueEvent(CommandEvent{PS_AutoIdentify, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return AutoIdentifyConfirmationResult::err(lastConfirmationCode);
    return AutoIdentifyConfirmationResult::ok({
        swapEndian(rxData.data[1]),
        swapEndian(static_cast<uint16_t>(rxData.data[2] | (rxData.data[3] << 8))),
        swapEndian(static_cast<uint16_t>(rxData.data[4] | (rxData.data[5] << 8)))
    });
}

ConfirmationResult SensorHiLinkZw0608::sleep() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_Sleep});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

GetChipSnConfirmationResult SensorHiLinkZw0608::getChipSN() {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        uint8_t parameter;
    } data = {0};
    enqueueEvent(CommandEvent{PS_GetChipSN, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return GetChipSnConfirmationResult::err(lastConfirmationCode);
    const auto ret = GetChipSnConfirmationResult::ok({});
    std::memcpy((void *) ret.value().chipSn, &rxData.data[1], 32);
    return ret;
}

ConfirmationResult SensorHiLinkZw0608::handShake() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_HandShake});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::checkSensor() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_CheckSensor});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::restSetting() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_RestSetting});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

ConfirmationResult SensorHiLinkZw0608::controlBLN(const ControlBLNFunction function, const ControlBLNColor startColor,
    const ControlBLNColor endColor, const uint8_t cycles) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        ControlBLNFunction function;
        ControlBLNColor startColor;
        ControlBLNColor endColor;
        uint8_t cycles;
    } data = {function, startColor, endColor, cycles};
    enqueueEvent(CommandEvent{PS_ControlBLN, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    return (lastConfirmationCode == Confirmation::Code::OK)
               ? ConfirmationResult::ok()
               : ConfirmationResult::err(lastConfirmationCode);
}

GetImageInfoConfirmationResult SensorHiLinkZw0608::getImageInfo() {
    clearReadyFlag();
    enqueueEvent(CommandEvent{PS_GetImageInfo});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return GetImageInfoConfirmationResult::err(lastConfirmationCode);
    return GetImageInfoConfirmationResult::ok({
        swapEndian(rxData.data[1]),
        swapEndian(rxData.data[2])
    });
}

SearchNowConfirmationResult SensorHiLinkZw0608::searchNow(const PageId startPage, const uint16_t pageCount) {
    clearReadyFlag();
    const struct [[gnu::packed]] data_t {
        PageId startPage;
        uint16_t pageCount;
    } data = {swapEndian(startPage), swapEndian(pageCount)};
    enqueueEvent(CommandEvent{PS_SearchNow, (uint8_t *) &data, sizeof(data)});
    awaitReadyFlag();
    if (lastConfirmationCode != Confirmation::Code::OK)
        return SearchNowConfirmationResult::err(lastConfirmationCode);
    return SearchNowConfirmationResult::ok({
        swapEndian(static_cast<uint16_t>(rxData.data[1] | (rxData.data[2] << 8))),
        swapEndian(static_cast<uint16_t>(rxData.data[3] | (rxData.data[4] << 8)))
    });
}
