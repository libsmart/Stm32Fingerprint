/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "../../../../src/libsmart_config.dist.hpp"
#include "../Lib/Stm32Common/src/libsmart_config.dist.hpp"
#include "../Lib/Stm32ItmLogger/src/libsmart_config.dist.hpp"
#include "../Lib/Stm32Serial/src/libsmart_config.dist.hpp"
#include "../Lib/Stm32ThreadX/src/libsmart_config.dist.hpp"
#include "../Lib/Stm32NetX/src/libsmart_config.dist.hpp"
#include "../Lib/Stm32Shell/src/libsmart_config.dist.hpp"


#undef LIBSMART_LOGGER_DEFAULT_PRINT_SEVERITY
// #define LIBSMART_LOGGER_DEFAULT_PRINT_SEVERITY Stm32ItmLogger::LoggerInterface::warningPrintSeverity
#define LIBSMART_LOGGER_DEFAULT_PRINT_SEVERITY Stm32ItmLogger::LoggerInterface::informationalPrintSeverity


#define LIBSMART_STM32SERIAL_ENABLE_HAL_UART_IT_DRIVER

#undef LIBSMART_STM32SERIAL_BUFFER_SIZE_TX
#define LIBSMART_STM32SERIAL_BUFFER_SIZE_TX (4*1024)

#undef LIBSMART_STM32SHELL_MICRORLSTREAMSESSION_BUFFER_SIZE_TX
#define LIBSMART_STM32SHELL_MICRORLSTREAMSESSION_BUFFER_SIZE_TX (1*1024)

// #undef LIBSMART_STM32NETX_NETX_THREAD_STACK_SIZE
// #define LIBSMART_STM32NETX_NETX_THREAD_STACK_SIZE (4*LIBSMART_STM32NETX_DEFAULT_MEMORY_SIZE)
