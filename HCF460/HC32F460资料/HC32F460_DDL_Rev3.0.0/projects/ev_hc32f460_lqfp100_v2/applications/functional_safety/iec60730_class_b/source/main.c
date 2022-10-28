/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/main.c
 * @brief main project.
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
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

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
 * @brief  Main function
 * @param  None
 * @retval None
 */
int32_t main(void)
{
    uint32_t u32LoopCaseTableSize;
    const stc_stl_case_runtime_t *pstlLoopCaseTable;

    /* BSP initialize */
    BSP_CLK_Init();

    /* STL initialize */
    pstlLoopCaseTable = TEST_GetRuntimeCaseTable();
    u32LoopCaseTableSize = TEST_GetRuntimeCaseTableSize();

    /* Initialize main loop test */
    STL_RuntimeTestInit(pstlLoopCaseTable, u32LoopCaseTableSize);

    /* Initialize period test */
    STL_RuntimeTestInit(TEST_GetPeriodCaseTable(),  TEST_GetPeriodCaseTableSize());

    /* Configure systick for period test */
    SysTick_Config(STL_SYSTICK_TICK_VALUE);

    for (;;) {
        /* Main loop test: interval time 30ms */
        if (g_u32SysTickCount >= 30UL) {
            g_u32SysTickCount = 0UL;
            STL_RuntimeTestCase(pstlLoopCaseTable, u32LoopCaseTableSize);
        } else {
            TEST_FeedWatchdog();
        }

        /* Add user code */
        {
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
