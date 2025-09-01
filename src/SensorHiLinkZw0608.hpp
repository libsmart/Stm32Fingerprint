/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <libsmart_config.hpp>
#include <main.h>

#include "PinDigitalIn.hpp"
#include "PinDigitalOut.hpp"
#include "Stm32Serial.hpp"
#include "FSM/ThreadXStateMachine.hpp"
#include "SensorEvents.hpp"
#include "SensorStates.hpp"
#include "States/PsReadInfPageState.hpp"

namespace Stm32Fingerprint {
    using SensorStateMachine = ThreadXStateMachine<
        Events_t,
        States::UninitializedState,
        States::InitializeState,
        States::ReadyState,
        States::CommandState,
        States::PsWriteRegState,
        States::PsReadSysParaState,
        States::PsReadInfPageState,
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
                  States::PsWriteRegState{"PS_WriteReg", this, &logger},
                  States::PsReadSysParaState{"PS_ReadSysPara", this, &logger},
                  States::PsReadInfPageState{"PS_ReadINFpage", this, &logger},
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
        friend States::PsWriteRegState;
        friend States::PsReadSysParaState;
        friend States::PsReadInfPageState;
        friend States::GetChipSnState;
        friend States::HandShakeState;
        friend States::ResetState;
        friend States::ErrorState;

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
    };
}
