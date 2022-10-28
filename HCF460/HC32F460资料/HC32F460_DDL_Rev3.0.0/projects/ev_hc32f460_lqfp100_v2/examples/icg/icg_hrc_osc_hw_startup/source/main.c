/**
 *******************************************************************************
 * @file  icg/icg_hrc_osc_hw_startup/source/main.c
 * @brief Main program of ICG HRC Frequency for the Device Driver Library.
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
 * @addtogroup ICG_HRC_Frequency
 * @{
 */

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

/* Clock output Port/Pin definition */
#define HRC_MCO_PORT                    (GPIO_PORT_E)
#define HRC_MCO_PIN                     (GPIO_PIN_00)
#define HRC_MCO_CH                      (CLK_MCO1)

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
 * @brief  HRC Clock output config.
 * @param  None
 * @retval None
 */
static void HRC_ClockOutputConfig(void)
{
    /* Configure clock output pin */
    GPIO_SetFunc(HRC_MCO_PORT, HRC_MCO_PIN, GPIO_FUNC_1);
    /* Configure clock output system clock */
    CLK_MCOConfig(HRC_MCO_CH, CLK_MCO_SRC_HRC, CLK_MCO_DIV1);
    CLK_MCOCmd(HRC_MCO_CH, ENABLE);
}

/**
 * @brief  Main function of ICG HRC Frequency.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    BSP_LED_Init();
    /* Configure clock output */
    HRC_ClockOutputConfig();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        BSP_LED_Toggle(LED_RED);
        DDL_DelayMS(1000UL);
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
