/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <main.h>
#include <libsmart_config.hpp>
#include <chrono>
#include "PinDigitalIn.hpp"
#include "PinDigitalOut.hpp"
#include "Stm32Serial.hpp"
#include "FSM/ThreadXStateMachine.hpp"
#include "SensorEvents.hpp"
#include "SensorStates.hpp"
#include "EventFlags/EventFlags.hpp"
#include "States/PsReadInfPageState.hpp"
#include "Types/Types.hpp"

namespace Stm32Fingerprint {
    using SensorStateMachine = ThreadXStateMachine<
        Events_t,
        States::UninitializedState,
        States::InitializeState,
        States::ReadyState,
        States::CommandState,
        States::PsUpCharState,
        States::PsUpImageState,
        States::PsDownImageState,
        States::PsReadSysParaState,
        States::PsReadInfPageState,
        States::PsAutoIdentifyState,
        States::GetChipSnState,
        States::HandShakeState,
        States::ResetState,
        States::ErrorState
    >;

    class SensorHiLinkZw0608 : public SensorStateMachine {
    public:
        SensorHiLinkZw0608(
            const char *name,
            Stm32Serial::Stm32Serial &serial,
            Stm32Gpio::PinDigitalIn &pin_detect,
            Stm32Gpio::PinDigitalOut &pin_enable,
            Stm32ItmLogger::LoggerInterface &logger
        )
            : SensorStateMachine(
                  States::UninitializedState{"UNINITIALIZED", this, &logger},
                  States::InitializeState{"INIT", this, &logger},
                  States::ReadyState{"READY", this, &logger},
                  States::CommandState{"CMD", this, &logger},
                  States::PsUpCharState{"PS_UpChar", this, &logger},
                  States::PsUpImageState{"PS_UpImage", this, &logger},
                  States::PsDownImageState{"PS_DownImage", this, &logger},
                  States::PsReadSysParaState{"PS_ReadSysPara", this, &logger},
                  States::PsReadInfPageState{"PS_ReadINFpage", this, &logger},
                  States::PsAutoIdentifyState{"PS_AutoIdentify", this, &logger},
                  States::GetChipSnState{"PS_GetChipSN", this, &logger},
                  States::HandShakeState{"PS_HandShake", this, &logger},
                  States::ResetState{"RESET", this, &logger},
                  States::ErrorState{"ERROR", this, &logger},
                  name, &logger
              ),
              pinDetect(pin_detect),
              pinEnable(pin_enable),
              serial(serial) { ; }

        friend States::UninitializedState;
        friend States::InitializeState;
        friend States::ReadyState;
        friend States::CommandState;
        friend States::PsUpCharState;
        friend States::PsUpImageState;
        friend States::PsDownImageState;
        friend States::PsReadSysParaState;
        friend States::PsReadInfPageState;
        friend States::PsAutoIdentifyState;
        friend States::GetChipSnState;
        friend States::HandShakeState;
        friend States::ResetState;
        friend States::ErrorState;

        static constexpr uint8_t swapEndian(const uint8_t val) { return val; };

        static constexpr uint16_t swapEndian(const uint16_t val) {
            return (val << 8) | (val >> 8);
        };

        static constexpr uint32_t swapEndian(const uint32_t val) {
            return ((val & 0x000000FFU) << 24) |
                   ((val & 0x0000FF00U) << 8) |
                   ((val & 0x00FF0000U) >> 8) |
                   ((val & 0xFF000000U) >> 24);
        };

        static constexpr uint64_t swapEndian(const uint64_t val) {
            return ((val & 0x00000000000000FFULL) << 56) |
                   ((val & 0x000000000000FF00ULL) << 40) |
                   ((val & 0x0000000000FF0000ULL) << 24) |
                   ((val & 0x00000000FF000000ULL) << 8) |
                   ((val & 0x000000FF00000000ULL) >> 8) |
                   ((val & 0x0000FF0000000000ULL) >> 24) |
                   ((val & 0x00FF000000000000ULL) >> 40) |
                   ((val & 0xFF00000000000000ULL) >> 56);
        };

        static constexpr auto be16 = [](const uint8_t *p) -> uint16_t {
            return (static_cast<uint16_t>(p[0]) << 8) | static_cast<uint16_t>(p[1]);
        };

        static constexpr auto be32 = [](const uint8_t *p) -> uint32_t {
            return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
                   (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
        };


        void setup() override;

        void initialize();

        void loop() override;

        void end() override;

        void errorHandler() override;

        void isrDetect();

    protected:
        uint16_t calc_checksum(uint8_t *data, int32_t start, int32_t end);

        bool check_checksum(uint8_t *data, uint32_t len);

        void parseReply();

    public:
        void sendCommand(uint8_t instruction);

        void sendCommand(uint8_t instruction, const uint8_t *data, uint16_t dataLength);

        void sendPacket(uint8_t packetId, const uint8_t *data, uint16_t dataLength);


        static constexpr size_t HEADER_SIZE = 2;
        static constexpr size_t ADDRESS_SIZE = 4;
        static constexpr size_t PID_SIZE = 1;
        static constexpr size_t LENGTH_SIZE = 2;
        static constexpr size_t MAX_DATA_SIZE = 256;
        static constexpr size_t CHECKSUM_SIZE = 2;

        static constexpr size_t DATA_OFFSET = HEADER_SIZE + ADDRESS_SIZE + PID_SIZE + LENGTH_SIZE;
        static constexpr size_t MAX_RX_FRAME_SIZE = DATA_OFFSET + MAX_DATA_SIZE + CHECKSUM_SIZE;
        static constexpr size_t MAX_TX_FRAME_SIZE = 64;

    private:
        Stm32Gpio::PinDigitalIn &pinDetect;
        Stm32Gpio::PinDigitalOut &pinEnable;
        Stm32Serial::Stm32Serial &serial;

        uint32_t address = 0xffffffff;

        using flags_t = enum class flags_t: ULONG {
            READY = 1 << 0,
            COMMAND = 1 << 1,
            PARSING = 1 << 2,
        };
        Stm32ThreadX::EventFlags flags{"flags"};

        uint8_t txFrame[MAX_TX_FRAME_SIZE]{};
        uint8_t rxFrame[MAX_RX_FRAME_SIZE]{};

        uint16_t rollingChecksum = 0;

        struct rxData_t {
            uint16_t header;
            uint32_t address;
            uint8_t packageId;
            uint16_t packetLength;
            uint8_t *data;
            uint16_t checksum;
        } rxData{};

        using parserState_t = enum class parserState_t : uint8_t {
            NONE = 0,
            HEADER,
            ADDRESS,
            PACKAGE_ID,
            PACKET_LENGTH,
            DATA,
            CHECKSUM,
            CHECK_CHECKSUM,
            DONE,
            ERROR
        };
        parserState_t parserState = parserState_t::NONE;
        size_t cnt = 0;
        size_t frameBytesParsed = 0;

    public:
        using Command = enum : uint8_t {
            NOP = 0,
            PS_GetImage = 0x01,
            PS_GenChar = 0x02,
            PS_Match = 0x03,
            PS_Search = 0x04,
            PS_RegModel = 0x05,
            PS_StoreChar = 0x06,
            PS_LoadChar = 0x07,
            PS_UpChar = 0x08,
            PS_DownChar = 0x09,
            PS_UpImage = 0x0a,
            PS_DownImage = 0x0b,
            PS_DeleteChar = 0x0c,
            PS_Empty = 0x0d,
            PS_WriteReg = 0x0e,
            PS_ReadSysPara = 0x0f,

            PS_SetPwd = 0x12,
            PS_VfyPwd = 0x13,
            PS_GetRandomCode = 0x14,
            PS_SetChipAddr = 0x15,
            PS_ReadINFpage = 0x16,

            PS_WriteNotepad = 0x18,
            PS_ReadNotepad = 0x19,

            PS_BurnCode = 0x1a,
            PS_ValidTemplateNum = 0x1d,
            PS_ReadIndexTable = 0x1f,

            PS_GetEnrollImage = 0x29,
            PS_Cancel = 0x30,
            PS_AutoEnroll = 0x31,
            PS_AutoIdentify = 0x32,
            PS_Sleep = 0x33,
            PS_GetChipSN = 0x34,
            PS_HandShake = 0x35,
            PS_CheckSensor = 0x36,

            PS_RestSetting = 0x3b,
            PS_ControlBLN = 0x3c,
            PS_GetImageInfo = 0x3d,
            PS_SearchNow = 0x3e,

            PS_GetKeyt = 0xe0,
            PS_LockKeyt = 0xe1,
            PS_GetCiphertext = 0xe2,
            PS_SecurityStoreChar = 0xe3,
            PS_SecuritySearch = 0xe4,


            NONE = 0xff
        };


        /**
         * @brief Captures an image using the HiLink ZW-0608 fingerprint sensor and returns the result of the operation.
         *
         * This function triggers the image acquisition process by sending a "Get Image" command to the sensor.
         * It ensures proper synchronization with the sensor's ready state and evaluates the confirmation
         * response to return the outcome of the operation.
         *
         * @return An object of type ConfirmationResult:
         *         - If the operation is successful, `ConfirmationResult::ok()` is returned.
         *         - If the operation fails, `ConfirmationResult::err(lastConfirmationCode)` is returned,
         *           containing the specific error code from the sensor.
         */
        ConfirmationResult getImage();

        ConfirmationResult genChar(BufferId bufferId);

        MatchConfirmationResult match();

        SearchConfirmationResult search(BufferId bufferId, uint16_t startPage, uint16_t countPage);

        ConfirmationResult regModel();

        ConfirmationResult storeChar(BufferId bufferId, PageId pageId);

        /**
         * @brief Loads a template from the flash memory of the HiLink ZW-0608 fingerprint sensor into a specified buffer.
         *
         * This function sends the "Load Character" command to the sensor, specifying the target buffer and the page ID
         * of the template in flash memory. It facilitates synchronization with the sensor and evaluates
         * the confirmation response to determine the outcome of the operation.
         *
         * @param bufferId The identifier of the buffer where the template will be loaded.
         * @param pageId The ID of the page in flash memory containing the desired template.
         * @return An object of type ConfirmationResult:
         *         - If the operation is successful, `ConfirmationResult::ok()` is returned.
         *         - If the operation fails, `ConfirmationResult::err(lastConfirmationCode)` is returned,
         *           containing the specific error code from the sensor.
         */
        ConfirmationResult loadChar(BufferId bufferId, PageId pageId);

        /**
         * @brief Uploads a character template from the fingerprint sensor's buffer to the host.
         *
         * Sends a request to the fingerprint sensor to retrieve a character template stored in the specified buffer.
         * The retrieved template is written into the provided PsUpCharEvent::template_t object. The method utilizes an
         * asynchronous mechanism for communication with the sensor and processes the confirmation response to determine
         * the result of the operation.
         *
         * @param bufferId The ID of the buffer containing the character template to be uploaded. This typically corresponds
         *                 to a predefined sensor buffer (e.g., BufferId::BufferA or BufferId::BufferB).
         * @param tpl Reference to a PsUpCharEvent::template_t object where the retrieved template data will be stored.
         *
         * @return An object of type ConfirmationResult:
         *         - If the operation is successful, `ConfirmationResult::ok()` is returned.
         *         - If the operation fails, `ConfirmationResult::err(lastConfirmationCode)` is returned, containing the
         *           specific error code from the sensor.
         */
        ConfirmationResult upChar(BufferId bufferId, PsUpCharEvent::template_t &tpl);

        ConfirmationResult downChar(BufferId bufferId, const PsUpCharEvent::template_t &tpl);

        ConfirmationResult upImage(PsUpImageEvent::image_t &image);

        ConfirmationResult downImage(PsDownImageEvent::image_t &image);

        ConfirmationResult deleteChar(PageId pageId, uint16_t count);

        ConfirmationResult empty();

        using register_t = enum class register_t : uint8_t {
            SERIAL_PORT_DELAY = 0,
            NUMBER_OF_REGISTRATIONS = 1,
            IMAGE_FORMAT = 2,
            REGISTER_LOGIC = 3,
            BAUD_RATE = 4,
            COMPARISON_THRESHOLD = 5,
            PACKET_SIZE = 6,
            ENCRYPTION_LEVEL = 7,
            ANTI_FAKE_FINGERPRINT = 8,
            SENSOR_PARAMETERS = 9,
        };

        ConfirmationResult writeReg(register_t reg, uint8_t content);

        ConfirmationResult readSysPara(PsReadSysParaEvent::sysPara_t &sysPara);

        ConfirmationResult setPwd(Password password);

        ConfirmationResult vfyPwd(Password password);

        GetRandomCodeConfirmationResult getRandomCode();

        ConfirmationResult setChipAddr(DeviceAddress deviceAddress);

        ReadInfPageConfirmationResult readInfPage();

        ConfirmationResult writeNotepad(NotepadPageId notepadPageId, NotepadPageContent content);

        ConfirmationResult readNotepad(NotepadPageId notepadPageId, NotepadPageContent &content);

        ConfirmationResult burnCode(UpgradeMode upgradeMode);

        ValidTemplateNumConfirmationResult validTemplateNum();

        ReadIndexTableConfirmationResult readIndexTable(IndexPageId indexPageId);

        ConfirmationResult getEnrollImage();

        ConfirmationResult cancel();

        AutoEnrollConfirmationResult autoEnroll(FingerprintId fingerprintId, uint8_t numberOfEntries,
                                                AutoEnrollParameter parameter);

        AutoIdentifyConfirmationResult autoIdentify(ScoreLevel scoreLevel, FingerprintId fingerprintId,
                                                    AutoIdentifyParameter parameter);

        ConfirmationResult sleep();

        GetChipSnConfirmationResult getChipSN();

        ConfirmationResult handShake();

        ConfirmationResult checkSensor();

        ConfirmationResult restSetting();

        ConfirmationResult controlBLN(ControlBLNFunction function, ControlBLNColor startColor, ControlBLNColor endColor,
                                      uint8_t cycles);

        GetImageInfoConfirmationResult getImageInfo();

        SearchNowConfirmationResult searchNow(PageId startPage, uint16_t pageCount);

    private:
        Confirmation lastConfirmationCode = Confirmation::Code::UNKNOWN;
        static constexpr uint32_t DEFAULT_WAIT{1000};

        void clearReadyFlag() {
            lastConfirmationCode = Confirmation::Code::UNKNOWN;
            flags.clear(static_cast<ULONG>(flags_t::READY));
        }

        void awaitReadyFlag(const uint32_t timeout = DEFAULT_WAIT) {
            const auto ret = flags.await(static_cast<ULONG>(flags_t::READY), {timeout});
            switch (ret) {
                case TX_SUCCESS: return;
                case TX_NO_EVENTS:
                    lastConfirmationCode = Confirmation::Code::ERROR_TIMEOUT;
                    return;
                default:
                    lastConfirmationCode = Confirmation::Code::ERROR_GENERIC;
            }
        }
    };
}
