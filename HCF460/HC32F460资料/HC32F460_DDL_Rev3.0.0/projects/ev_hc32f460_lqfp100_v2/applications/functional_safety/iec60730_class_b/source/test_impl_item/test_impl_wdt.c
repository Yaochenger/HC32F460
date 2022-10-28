/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/test_impl_item/test_impl_wdt.c
 * @brief This file provides firmware functions to implement the watch test.
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
#include "hc32_ll.h"
#include "stl_utility.h"
#include "test_impl_wdt.h"

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @defgroup Test_Implement_WDT Test Implement Watchdog
 * @{
 */

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define WDT_TIMEOUT                 (1000UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @defgroup Test_Implement_WDT_Global_Functions Test Implement Watchdog Global Functions
 * @{
 */

/**
 * @brief  Watchdog test initialize in runtime.
 * @param  [in] u32CountPeriod  The WDT counting period value @ref WDT_Count_Period
 * @param  [in] u32ClockDiv     The WDT clock division factor @ref WDT_Clock_Division
 * @retval uint32_t:
 *           - STL_OK:          Initialize successfully.
 *           - STL_ERR:         Initialize unsuccessfully.
 */
static uint32_t TEST_IMPL_WdtInit(uint32_t u32CountPeriod, uint32_t u32ClockDiv)
{
    stc_wdt_init_t stcWdtInit;

    /* WDT configuration */
    stcWdtInit.u32CountPeriod = u32CountPeriod;
    stcWdtInit.u32ClockDiv = u32ClockDiv;
    stcWdtInit.u32RefreshRange = WDT_RANGE_0TO100PCT;
    stcWdtInit.u32LPMCount = WDT_LPM_CNT_STOP;
    stcWdtInit.u32ExceptionType = WDT_EXP_TYPE_RST;
    (void)WDT_Init(&stcWdtInit);

    /* First reload counter to start WDT */
    WDT_FeedDog();

    return STL_OK;
}

/**
 * @brief  Watchdog test in startup.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Test pass.
 *           - STL_ERR:         Test fail.
 */
uint32_t STL_WdtStartupTest(void)
{
    uint32_t i;
    uint32_t u32Ret;

    if (SET == RMU_GetStatus(RMU_FLAG_WDT)) {
        RMU_ClearStatus();
        u32Ret = STL_OK;
    } else {
        (void)TEST_IMPL_WdtInit(WDT_CNT_PERIOD256, WDT_CLK_DIV4);

        for (i = 0UL; i < WDT_TIMEOUT; i++) {
            STL_DelayMS(1UL);
        }
        u32Ret = STL_ERR;
    }

    return u32Ret;
}

/**
 * @brief  Watchdog test initialize in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Initialize successfully.
 *           - STL_ERR:         Initialize unsuccessfully.
 */
uint32_t STL_WdtRuntimeInit(void)
{
    return TEST_IMPL_WdtInit(WDT_CNT_PERIOD16384, WDT_CLK_DIV128);
}

/**
 * @brief  Watchdog feed in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Test pass.
 *           - STL_ERR:         Test fail.
 */
uint32_t STL_WdtRuntimeFeed(void)
{
    WDT_FeedDog();
    return STL_OK;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
