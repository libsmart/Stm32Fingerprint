/*
 * SPDX-FileCopyrightText: 2025 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "InterruptHandlerCallback.hpp"
#include "globals.hpp"
#include "Helper.hpp"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    LIBSMART_UNUSED(GPIO_Pin);

    switch (GPIO_Pin) {

        case FP_DETECT_Pin: // EXTI3
             Logger.printf("HAL_GPIO_EXTI_Callback(%d): FP_DETECT_Pin \n", GPIO_Pin);
#if ENABLE_FP==1
            fpSensor.isrDetect();
#endif
            break;


        default:
            Logger.printf("HAL_GPIO_EXTI_Callback(%d): UNKNOWN \n", GPIO_Pin);
    }
}

/*
void HAL_IWDG_EarlyWakeupCallback(IWDG_HandleTypeDef *hiwdg) {
    LIBSMART_UNUSED(hiwdg);
    // Logger.printf("HAL_IWDG_EarlyWakeupCallback()\n");
    errorHandler();
}
*/
