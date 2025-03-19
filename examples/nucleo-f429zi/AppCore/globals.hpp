/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/**
 * This file holds exports for the global variables, defined in globals.cpp.
 * @see globals.cpp
 */

#ifndef NUCLEO_F429ZI_APPCORE_GLOBALS_HPP
#define NUCLEO_F429ZI_APPCORE_GLOBALS_HPP

#include "Stm32ItmLogger.hpp"
#include <cstdint>

#include "PinDigitalIn.hpp"
#include "PinDigitalOut.hpp"
#include "Dns/Dns.hpp"
#include "Driver/Stm32HalUartItDriver.hpp"
#include "ezShell/Shell.hpp"
#include "StreamSession/GeneralStreamSession.hpp"

extern "C" {
#include "globals.h"
#include "usart.h"
}

extern uint32_t dummyCpp;

/**
 * Global logger instance
 */
inline Stm32ItmLogger::Stm32ItmLogger &Logger = Stm32ItmLogger::logger;

/* ezShell over serial */
inline Stm32Common::StreamSession::Manager<Stm32Shell::ezShell::Shell, 1> microrlStreamSessionManager(&Logger);
// inline Stm32Common::StreamSession::Manager<Stm32Common::StreamSession::EchoStreamSession, 1> serial4SessionManager;
inline Stm32Serial::Stm32HalUartItDriver uart3Driver(&huart3, "uart3Driver");
inline Stm32Serial::Stm32Serial Serial3(&uart3Driver, &microrlStreamSessionManager);


// Fingerprint (Hi-Link ZW0608)
#if ENABLE_FP==1
inline Stm32Gpio::PinDigitalIn fp_DETECT("FP_DETECT", FP_DETECT_GPIO_Port, FP_DETECT_Pin);
inline Stm32Gpio::PinDigitalOut fp_nSTDBY("FP_nSTDBY", FP_nSTDBY_GPIO_Port, FP_nSTDBY_Pin);

inline Stm32Common::StreamSession::Manager<Stm32Common::StreamSession::GeneralStreamSession, 1>
fpSessionManager(&Logger);
inline Stm32Serial::Stm32HalUartItDriver uart2Driver(&huart2, "uart2Driver");
inline Stm32Serial::Stm32Serial Serial2(&uart2Driver, &fpSessionManager);

// inline Stm32Fingerprint::SensorHiLinkZw0608 fpSensor("FP", Serial7, fp_DETECT, fp_nSTDBY, Logger);
inline uint8_t fpSensorThreadStack[4 * 1024];
#endif

#endif
