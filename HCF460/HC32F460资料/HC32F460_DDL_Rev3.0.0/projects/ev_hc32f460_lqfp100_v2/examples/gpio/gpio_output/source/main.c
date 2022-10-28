/**
 *******************************************************************************
 * @file  gpio/gpio_output/source/main.c
 * @brief Main program of GPIO for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2022, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "main.h"

/**
 * @addtogroup HC32F460_DDL_Examples
 * @{
 */

/**
 * @addtogroup GPIO_OUTPUT
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* LED_R Port/Pin definition */
#define LED_R_PORT          (GPIO_PORT_D)
#define LED_R_PIN           (GPIO_PIN_03)
/* LED_G Port/Pin definition */
#define LED_G_PORT          (GPIO_PORT_D)
#define LED_G_PIN           (GPIO_PIN_04)
/* LED_Y Port/Pin definition */
#define LED_Y_PORT          (GPIO_PORT_D)
#define LED_Y_PIN           (GPIO_PIN_05)
/* LED_B Port/Pin definition */
#define LED_B_PORT          (GPIO_PORT_D)
#define LED_B_PIN           (GPIO_PIN_06)
/* LED toggle definition */
#define LED_R_TOGGLE()      (GPIO_TogglePins(LED_R_PORT, LED_R_PIN))
#define LED_G_TOGGLE()      (GPIO_TogglePins(LED_G_PORT, LED_G_PIN))
#define LED_Y_TOGGLE()      (GPIO_TogglePins(LED_Y_PORT, LED_Y_PIN))
#define LED_B_TOGGLE()      (GPIO_TogglePins(LED_B_PORT, LED_B_PIN))

#define DLY_MS              (100UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  LED Init
 * @param  None
 * @retval None
 */
static void LED_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinState = PIN_STAT_RST;
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(LED_R_PORT, LED_R_PIN, &stcGpioInit);
    (void)GPIO_Init(LED_G_PORT, LED_G_PIN, &stcGpioInit);
    (void)GPIO_Init(LED_Y_PORT, LED_Y_PIN, &stcGpioInit);
    (void)GPIO_Init(LED_B_PORT, LED_B_PIN, &stcGpioInit);
}

/**
 * @brief  Main function of GPIO project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO);
    /* LED initialize */
    LED_Init();
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO);
    for (;;) {
        LED_R_TOGGLE();
        DDL_DelayMS(DLY_MS);
        LED_G_TOGGLE();
        DDL_DelayMS(DLY_MS);
        LED_Y_TOGGLE();
        DDL_DelayMS(DLY_MS);
        LED_B_TOGGLE();
        DDL_DelayMS(DLY_MS);
        /* De-init port if necessary */
        // GPIO_DeInit();
    }
}

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
