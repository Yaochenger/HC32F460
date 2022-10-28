/**
 *******************************************************************************
 * @file  wdt/wdt_reset_sw_startup/source/main.c
 * @brief Main program of WDT Reset for the Device Driver Library.
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
 * @addtogroup WDT_Reset
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

/* WDT count period definition */
#define WDT_COUNT_PERIOD                (16384U)

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
 * @brief  WDT configuration.
 * @param  None
 * @retval None
 */
static void WDT_Config(void)
{
    stc_wdt_init_t stcWdtInit;

    /* WDT configuration */
    stcWdtInit.u32CountPeriod   = WDT_CNT_PERIOD16384;
    stcWdtInit.u32ClockDiv      = WDT_CLK_DIV512;
    stcWdtInit.u32RefreshRange  = WDT_RANGE_0TO25PCT;
    stcWdtInit.u32LPMCount      = WDT_LPM_CNT_STOP;
    stcWdtInit.u32ExceptionType = WDT_EXP_TYPE_RST;
    (void)WDT_Init(&stcWdtInit);
}

/**
 * @brief  Main function of WDT Reset.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint16_t u16CmpVal;
    uint32_t u32ResetSrc = 0UL;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Get RMU information */
    if (SET == RMU_GetStatus(RMU_FLAG_WDT)) {
        u32ResetSrc = RMU_FLAG_WDT;
        BSP_LED_On(LED_RED);
    }
    RMU_ClearStatus();
    /* Configure WDT */
    WDT_Config();
    /* First reload counter to start WDT */
    WDT_FeedDog();
    /* Wait for WDT module to complete initial */
    DDL_DelayMS(10U);
    /* Count period=16384, range=0%-25% */
    u16CmpVal = WDT_COUNT_PERIOD / 4U;
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_10)) {
            u16CmpVal = WDT_COUNT_PERIOD / 2U;
        }

        if (WDT_GetCountValue() < u16CmpVal) {
            //WDT_FeedDog();
            /* Wait for the count value to update */
            DDL_DelayMS(10U);
            if (0UL == u32ResetSrc) {
                BSP_LED_Toggle(LED_BLUE);
            }
        }
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
