/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef AT_FIRMWARE_APPCORE_INTERRUPTHANDLERCALLBACK_HPP
#define AT_FIRMWARE_APPCORE_INTERRUPTHANDLERCALLBACK_HPP

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif
