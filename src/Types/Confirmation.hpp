/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <array>
#include <cstdint>

#include "Types.hpp"
#include "Result/Result.hpp"

namespace Stm32Fingerprint {
    struct Confirmation {
        enum class Code : uint8_t {
            OK = 0x00,
            ERROR_RECEIVING_PACKAGE = 0x01,
            ERROR_NO_FINGER_ON_SENSOR = 0x02,
            ERROR_IMAGE_TOO_MESSY = 0x06,
            ERROR_TOO_FEW_FEATURE_POINTS = 0x07,
            ERROR_FINGERPRINT_DO_NOT_MATCH = 0x08,
            ERROR_NOT_FOUND = 0x09,
            ERROR_MERGE_FAILED = 0x0a,
            ERROR_ID_OUT_OF_RANGE = 0x0b,
            ERROR_TEMPLATE_INVALID = 0x0c,
            ERROR_INSTRUCTION_FAILED = 0x0d,
            ERROR_SUBSEQUENT_PACKETS_NOT_AVAILABLE = 0x0e,
            ERROR_TEMPLATE_DELETION_FAILED = 0x10,
            ERROR_CLEARING_DATABASE = 0x11,
            ERROR_PASSWORD_INCORRECT = 0x13,
            ERROR_NO_IMAGE_IN_BUFFER = 0x15,
            ERROR_RESIDUAL_FINGERPRINTS = 0x17,
            ERROR_FLASH_RW = 0x18,
            ERROR_RANDOM_NUMBER_FAILED = 0x19,
            ERROR_REGISTER_NUMBER_INVALID = 0x1a,
            ERROR_SETTING_CONTENT = 0x1b,
            ERROR_NOTEPAD_PAGE_INCORRECT = 0x1c,
            ERROR_DATABASE_FULL = 0x1f,
            ERROR_TEMPLATE_NOT_EMPTY = 0x22,
            ERROR_TEMPLATE_EMPTY = 0x23,
            ERROR_DATABASE_EMPTY = 0x24,
            ERROR_INCORRECT_NUMBER_OF_ENTRIES = 0x25,
            ERROR_TIMEOUT = 0x26,
            ERROR_FINGERPRINT_EXISTS = 0x27,
            ERROR_FEATURES_CORRELATE = 0x28,
            ERROR_SENSOR_FAILED = 0x29,
            ERROR_SECRET_KEY_DOES_NOT_EXIST = 0x2e,
            ERROR_ENCRYPTION_LEVEL_MISMATCH = 0x31,
            ERROR_IMAGE_AREA_SMALL = 0x33,
            ERROR_ILLEGAL_DATA = 0x35,
            ERROR_GENERIC = 0xfc,
            ERROR_NOT_IMPLEMENTED = 0xfe,
            UNKNOWN = 0xff
        };

        constexpr Confirmation() : Confirmation(Code::OK) { ; }

        constexpr Confirmation(const Code code) : value(code) { ; }

        constexpr operator Code() const { return value; }

        constexpr const char *to_string() const {
            for (const auto &e: entries) {
                if (e.value == value) return e.name;
            }
            return "UNDEFINED";
        }

    private:
        Code value;

        struct Entry {
            Code value;
            const char *name;
        };

        static constexpr Entry entries[]{
            Entry{Code::OK, "OK"},
            Entry{Code::ERROR_RECEIVING_PACKAGE, "ERROR_RECEIVING_PACKAGE"},
            Entry{Code::ERROR_NO_FINGER_ON_SENSOR, "ERROR_NO_FINGER_ON_SENSOR"},
            Entry{Code::ERROR_IMAGE_TOO_MESSY, "ERROR_IMAGE_TOO_MESSY"},
            Entry{Code::ERROR_TOO_FEW_FEATURE_POINTS, "ERROR_TOO_FEW_FEATURE_POINTS"},
            Entry{Code::ERROR_FINGERPRINT_DO_NOT_MATCH, "ERROR_FINGERPRINT_DO_NOT_MATCH"},
            Entry{Code::ERROR_NOT_FOUND, "ERROR_NOT_FOUND"},
            Entry{Code::ERROR_MERGE_FAILED, "ERROR_MERGE_FAILED"},
            Entry{Code::ERROR_ID_OUT_OF_RANGE, "ERROR_PAGEID_INVALID"},
            Entry{Code::ERROR_TEMPLATE_INVALID, "ERROR_TEMPLATE_INVALID"},
            Entry{Code::ERROR_INSTRUCTION_FAILED, "ERROR_INSTRUCTION_FAILED"},
            Entry{Code::ERROR_SUBSEQUENT_PACKETS_NOT_AVAILABLE, "ERROR_SUBSEQUENT_PACKETS_NOT_AVAILABLE"},
            Entry{Code::ERROR_TEMPLATE_DELETION_FAILED, "ERROR_TEMPLATE_DELETION_FAILED"},
            Entry{Code::ERROR_CLEARING_DATABASE, "ERROR_CLEARING_DATABASE"},
            Entry{Code::ERROR_PASSWORD_INCORRECT, "ERROR_PASSWORD_INCORRECT"},
            Entry{Code::ERROR_NO_IMAGE_IN_BUFFER, "ERROR_NO_IMAGE_IN_BUFFER"},
            Entry{Code::ERROR_RESIDUAL_FINGERPRINTS, "ERROR_NOT_MOVED"},
            Entry{Code::ERROR_FLASH_RW, "ERROR_FLASH_RW"},
            Entry{Code::ERROR_RANDOM_NUMBER_FAILED, "ERROR_RANDOM_NUMBER_FAILED"},
            Entry{Code::ERROR_REGISTER_NUMBER_INVALID, "ERROR_REGISTER_NUMBER_INVALID"},
            Entry{Code::ERROR_SETTING_CONTENT, "ERROR_SETTING_CONTENT"},
            Entry{Code::ERROR_NOTEPAD_PAGE_INCORRECT, "ERROR_NOTEPAD_PAGE_INCORRECT"},
            Entry{Code::ERROR_DATABASE_FULL, "ERROR_DATABASE_FULL"},
            Entry{Code::ERROR_TEMPLATE_NOT_EMPTY, "ERROR_TEMPLATE_NOT_EMPTY"},
            Entry{Code::ERROR_TEMPLATE_EMPTY, "ERROR_TEMPLATE_EMPTY"},
            Entry{Code::ERROR_DATABASE_EMPTY, "ERROR_DATABASE_EMPTY"},
            Entry{Code::ERROR_INCORRECT_NUMBER_OF_ENTRIES, "ERROR_INCORRECT_NUMBER_OF_ENTRIES"},
            Entry{Code::ERROR_TIMEOUT, "ERROR_TIMEOUT"},
            Entry{Code::ERROR_FINGERPRINT_EXISTS, "ERROR_FINGERPRINT_EXISTS"},
            Entry{Code::ERROR_FEATURES_CORRELATE, "ERROR_FEATURES_CORRELATE"},
            Entry{Code::ERROR_SENSOR_FAILED, "ERROR_SENSOR_FAILED"},
            Entry{Code::ERROR_SECRET_KEY_DOES_NOT_EXIST, "ERROR_SECRET_KEY_DOES_NOT_EXIST"},
            Entry{Code::ERROR_ENCRYPTION_LEVEL_MISMATCH, "ERROR_ENCRYPTION_LEVEL_MISMATCH"},
            Entry{Code::ERROR_IMAGE_AREA_SMALL, "ERROR_IMAGE_AREA_SMALL"},
            Entry{Code::ERROR_ILLEGAL_DATA, "ERROR_ILLEGAL_DATA"},
            Entry{Code::ERROR_GENERIC, "ERROR_GENERIC"},
            Entry{Code::ERROR_NOT_IMPLEMENTED, "ERROR_NOT_IMPLEMENTED"},
            Entry{Code::UNKNOWN, "UNKNOWN"}
        };
    };


    using ConfirmationResult = Stm32Common::Result<void, Confirmation>;
    using MatchConfirmationResult = Stm32Common::Result<MatchResult, Confirmation>;
    using SearchConfirmationResult = Stm32Common::Result<SearchResult, Confirmation>;
    using GetRandomCodeConfirmationResult = Stm32Common::Result<GetRandomCodeResult, Confirmation>;
    using ReadInfPageConfirmationResult = Stm32Common::Result<ReadInfPageResult, Confirmation>;
    using ValidTemplateNumConfirmationResult = Stm32Common::Result<ValidTemplateNumResult, Confirmation>;
    using ReadIndexTableConfirmationResult = Stm32Common::Result<ReadIndexTableResult, Confirmation>;
    using AutoEnrollConfirmationResult = Stm32Common::Result<AutoEnrollResult, Confirmation>;
    using AutoIdentifyConfirmationResult = Stm32Common::Result<AutoIdentifyResult, Confirmation>;
    using GetChipSnConfirmationResult = Stm32Common::Result<GetChipSnResult, Confirmation>;
    using GetImageInfoConfirmationResult = Stm32Common::Result<GetImageInfoResult, Confirmation>;
    using SearchNowConfirmationResult = Stm32Common::Result<SearchNowResult, Confirmation>;
}
