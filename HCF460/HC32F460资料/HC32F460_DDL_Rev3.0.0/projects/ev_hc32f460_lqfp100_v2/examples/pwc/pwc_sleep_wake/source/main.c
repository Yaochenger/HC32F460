/**
 *******************************************************************************
 * @file  pwc/pwc_sleep_wake/source/main.c
 * @brief Main program of PWC for the Device Driver Library.
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
 * @addtogroup PWC_Sleep_wake
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define DLY_MS          (500UL)

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
 * @brief  KEY10 External interrupt Ch.0 callback function
 *         IRQ No.0 in Global IRQ entry No.0~31 is used for EXTINT0
 * @param  None
 * @retval None
 */
void BSP_KEY_KEY10_IrqHandler(void)
{
    if (SET == EXTINT_GetExtIntStatus(BSP_KEY_KEY10_EXTINT)) {
        BSP_LED_Off(LED_RED);
        EXTINT_ClearExtIntStatus(BSP_KEY_KEY10_EXTINT);
    }
}

/**
 * @brief  Main function of sleep mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t u8Count;

    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System Clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();
    /* Key init */
    BSP_KEY_Init();

    /* KEY10 */
    while (PIN_RESET != GPIO_ReadInputPins(BSP_KEY_KEY10_PORT, BSP_KEY_KEY10_PIN)) {
        ;
    }
    DDL_DelayMS(DLY_MS);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    for (;;) {
        u8Count = 10U;
        do {
            BSP_LED_Toggle(LED_BLUE);
            DDL_DelayMS(DLY_MS);
        } while ((--u8Count) != 0U);
        BSP_LED_On(LED_RED);
        PWC_SLEEP_Enter();
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
