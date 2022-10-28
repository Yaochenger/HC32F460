/**
 *******************************************************************************
 * @file  execute_inplace/qspi_xip/source/main.c
 * @brief Main program of QSPI XIP for the Device Driver Library.
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

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

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
 * @brief  SysTick interrupt handler function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    SysTick_IncTick();
}

/**
 * @brief  Led function.
 * @param  None
 * @retval None
 */
#if defined (__GNUC__) && !defined (__CC_ARM)
__attribute__((section(".ex_rom"), noinline))
#elif defined (__CC_ARM)
__attribute__((used)) __attribute__((section(".ex_rom")))
#elif defined (__ICCARM__)
#pragma location = ".ex_rom"
#endif
void ExROM_LedFunc(void)
{
    BSP_LED_Toggle(LED_RED);
    SysTick_Delay(500U);
    BSP_LED_Toggle(LED_BLUE);
    SysTick_Delay(500U);
}

/**
 * @brief  Main function of QSPI XIP.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_LED_Init();
    /* SysTick configuration */
    (void)SysTick_Init(1000U);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        ExROM_LedFunc();
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
