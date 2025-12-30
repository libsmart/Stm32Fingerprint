/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */


#pragma once
/* NOT IN USE
#include "SensorHiLinkZw0608.hpp"
#include "Enum/EnumRegistry.hpp"
#include "Hash/Fnv1a.hpp"

namespace Stm32Fingerprint {
    struct Command {
        enum class Id : uint8_t {
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


        struct Meta {
            Id command;
            uint8_t id;
            Stm32Common::Hash::FNV1a::fnv1a32_t strHash;
            const char *name;
        };

#define DATA_LINE(x) { Id::x, static_cast<uint8_t>(Id::x), \
Stm32Common::Hash::FNV1a::hash(#x), \
#x \
}

        static constexpr Meta data[] = {
            {Id::NOP, static_cast<uint8_t>(Id::NOP), Stm32Common::Hash::FNV1a::hash("NOP"), "NOP"},
            DATA_LINE(PS_GetImage),
            DATA_LINE(PS_GenChar),
            DATA_LINE(PS_Match),
            DATA_LINE(PS_Search),
            DATA_LINE(PS_RegModel),
            DATA_LINE(PS_StoreChar),
            DATA_LINE(PS_LoadChar),
            DATA_LINE(PS_UpChar),
            DATA_LINE(PS_DownChar),
            DATA_LINE(PS_UpImage),
            DATA_LINE(PS_DownImage),
            DATA_LINE(PS_DeleteChar),
            DATA_LINE(PS_Empty),
            DATA_LINE(PS_WriteReg),
            DATA_LINE(PS_ReadSysPara),
            DATA_LINE(PS_SetPwd),
            DATA_LINE(PS_VfyPwd),
            DATA_LINE(PS_GetRandomCode),
            DATA_LINE(PS_SetChipAddr),
            DATA_LINE(PS_ReadINFpage),
            DATA_LINE(PS_WriteNotepad),
            DATA_LINE(PS_ReadNotepad),
            DATA_LINE(PS_BurnCode),
            DATA_LINE(PS_ValidTemplateNum),
            DATA_LINE(PS_ReadIndexTable),
            DATA_LINE(PS_GetEnrollImage),
            DATA_LINE(PS_Cancel),
            DATA_LINE(PS_AutoEnroll),
            DATA_LINE(PS_AutoIdentify),
            DATA_LINE(PS_Sleep),
            DATA_LINE(PS_GetChipSN),
            DATA_LINE(PS_HandShake),
            DATA_LINE(PS_CheckSensor),
            DATA_LINE(PS_RestSetting),
            DATA_LINE(PS_ControlBLN),
            DATA_LINE(PS_GetImageInfo),
            DATA_LINE(PS_SearchNow),
            DATA_LINE(PS_GetKeyt),
            DATA_LINE(PS_LockKeyt),
            DATA_LINE(PS_GetCiphertext),
            DATA_LINE(PS_SecurityStoreChar),
            DATA_LINE(PS_SecuritySearch),
        };

        static constexpr Stm32Common::EnumRegistry registry{data};

        using value = Stm32Common::RegistryView<Meta>;
    };
}
*/