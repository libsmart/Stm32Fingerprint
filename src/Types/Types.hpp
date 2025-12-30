/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <main.h>
#include <PrintInterface.hpp>

namespace Stm32Fingerprint {
    using BufferId = uint8_t;
    using PageId = uint16_t;
    using MatchScore = uint16_t;
    struct SearchResult {
        PageId pageId;
        MatchScore matchScore;
    };
    struct MatchResult {
        MatchScore matchScore;
    };
    using Password = uint32_t;
    using RandomNumber = uint32_t;
    struct GetRandomCodeResult {
        RandomNumber randomNumber;
    };
    using DeviceAddress = uint32_t;
    struct ReadInfPageResult {
        uint16_t enrollTimes;
        uint16_t templateSize;
        uint16_t databaseCapacity;
        uint16_t scoreLevel;
        DeviceAddress deviceAddress;
        uint16_t packetSize;
        uint16_t baudRate;
        uint16_t antiFakeFingerPrint;
        uint16_t sensorParameters;
        uint16_t encryptionLevel;
        uint16_t enrollLogic;
        uint16_t imageFormat;
        uint16_t serialPortDelay;
        uint8_t productSn[8];
        uint8_t softwareVersion[8];
        uint8_t manufacturer[8];
        uint8_t sensorName[8];
        Password password;
        uint32_t jtagLockFlag;
        uint16_t res1;
        uint16_t res2;
        uint16_t res3;
        uint16_t validFlag;
    };
    using NotepadPageId = uint8_t;
    using NotepadPageContent = uint8_t[32];
    using UpgradeMode = uint8_t;
    struct ValidTemplateNumResult {
        uint16_t validTemplateNum;
    };
    using IndexPageId = uint8_t;
    struct ReadIndexTableResult {
        uint8_t index[32];
    };
    using FingerprintId = uint16_t;
    struct AutoEnrollResult {
        uint8_t parameter1;
        uint8_t parameter2;
    };
    enum class AutoEnrollParameter : uint16_t {
        BACKLIGHT_CONTROL = 1 << 0,
        IMAGE_PREPROCESSING = 1 << 1,
        RETURN_STATUS = 1 << 2,
        ALLOW_ID_OVERWRITE = 1 << 3,
        ALLOW_REPEATED_ENROLLMENT = 1 << 4,
        FINGER_LEAVE_REQUIRED = 1 << 5
        // All others reserved
    };
    using ScoreLevel = uint8_t;
    using Score = uint16_t;
    enum class AutoIdentifyParameter : uint16_t {
        BACKLIGHT_CONTROL = 1 << 0,
        IMAGE_PREPROCESSING = 1 << 1,
        RETURN_STATUS = 1 << 2
    };
    struct AutoIdentifyResult {
        uint8_t parameter;
        FingerprintId fingerprintId;
        Score score;
    };
    using ChipSn = uint8_t[32];
    struct GetChipSnResult {
        ChipSn chipSn;
    };
    enum class ControlBLNFunction : uint8_t {
        BREATHING = 1,
        FLASHING = 2,
        NORMALLY_OPEN = 3,
        NORMALLY_CLOSE = 4,
        GRADUALLY_OPEN = 5,
        GRADUALLY_CLOSE = 6
    };
    enum class ControlBLNColor : uint8_t {
        BLUE = 1 << 0,
        GREEN = 1 << 1,
        RED = 1 << 2
    };
    struct GetImageInfoResult {
        uint8_t imageArea;
        uint8_t imageQuality;
    };
    struct SearchNowResult {
        PageId pageId;
        MatchScore matchScore;
    };
}
