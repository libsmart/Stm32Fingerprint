/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "InterruptHandlerCallback.hpp"
#include "globals.hpp"
#include "Helper.hpp"

extern "C" {
#include "st25r3916_irq.h"

extern void tc_isr();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    LIBSMART_UNUSED(GPIO_Pin);

    switch (GPIO_Pin) {
        case WiFi_FLOW_Pin: // EXTI1
            Logger.printf("HAL_GPIO_EXTI_Callback(%d): WiFi_FLOW_Pin \n", GPIO_Pin);
            break;

        case FP_DETECT_Pin: // EXTI2
            // Logger.printf("HAL_GPIO_EXTI_Callback(%d): FP_DETECT_Pin \n", GPIO_Pin);
#if FINGERPRINT==1
            fpSensor.isrDetect();
#endif
            break;

        /*
        case WiFi_NOTIFY_Pin: // EXTI3
            Logger.printf("HAL_GPIO_EXTI_Callback(%d): WiFi_NOTIFY_Pin \n", GPIO_Pin);
            break;
            */

        case LCD_INT_Pin: // EXTI3
            // Logger.printf("HAL_GPIO_EXTI_Callback(%d): LCD_INT_Pin \n", GPIO_Pin);
            tc_isr();
            break;

        case NFC_INT_Pin: // EXTI4
            // Logger.printf("HAL_GPIO_EXTI_Callback(%d): NFC_INT_Pin \n", GPIO_Pin);
            // runNfcIsr = true;
            if (i2c1.isReady()) {
                st25r3916Isr();
            } else {
                // __HAL_GPIO_EXTI_GENERATE_SWIT(GPIO_Pin);
            }
        // st25r3916Isr();
            break;

        case TOF_INT_Pin: // EXTI6
            // Logger.printf("HAL_GPIO_EXTI_Callback(%d): TOF_INT_Pin \n", GPIO_Pin);
            break;

        case UCPD1_INT_Pin: // EXTI8
            Logger.printf("HAL_GPIO_EXTI_Callback(%d): UCPD1_INT_Pin \n", GPIO_Pin);
            break;

        case EM4095_CLK_Pin: // EXTI12
            // Logger.printf("HAL_GPIO_EXTI_Callback(%d): EM4095_CLK_Pin \n", GPIO_Pin);
            rfid.isrClk();
            break;

        // case USER_BUTTON_Pin: // EXTI13
        //     Logger.printf("HAL_GPIO_EXTI_Callback(%d): USER_BUTTON_Pin \n", GPIO_Pin);
        //     if (userButton.isOn()) {
        //         // rising edge
        //         lcdBacklight.toggle();
        //         fpSensor.handShake();
        //     }
        //     if (userButton.isOff()) {
        //         // falling edge
        //     }
        //     break;

        case AUDIO_INT_Pin: // EXTI13
            Logger.printf("HAL_GPIO_EXTI_Callback(%d): AUDIO_INT_Pin \n", GPIO_Pin);
            break;

        case SD_DETECT_Pin: // EXTI14
            Logger.printf("HAL_GPIO_EXTI_Callback(%d): SD_DETECT_Pin \n", GPIO_Pin);
            break;

        case EM4095_DEMOD_Pin: // EXTI15
            // Logger.printf("HAL_GPIO_EXTI_Callback(%d): EM4095_DEMOD_Pin \n", GPIO_Pin);
            rfid.isrDemod();
            break;

        default:
            Logger.printf("HAL_GPIO_EXTI_Callback(%d): UNKNOWN \n", GPIO_Pin);
    }
}

void HAL_IWDG_EarlyWakeupCallback(IWDG_HandleTypeDef *hiwdg) {
    LIBSMART_UNUSED(hiwdg);
    // Logger.printf("HAL_IWDG_EarlyWakeupCallback()\n");
    errorHandler();
}
