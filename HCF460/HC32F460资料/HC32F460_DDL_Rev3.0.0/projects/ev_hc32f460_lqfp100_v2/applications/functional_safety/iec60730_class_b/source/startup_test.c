/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/startup_test.c
 * @brief IEC60730 class B for the startup test.
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
#include "stl_bsp_conf.h"
#include "stl_utility.h"
#include "stl_test_cpu.h"
#include "stl_test_pc.h"
#include "stl_test_ram.h"
#include "stl_test_flash.h"
#include "stl_test_startup.h"
#include "test_impl_wdt.h"

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @addtogroup Startup_Test
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define FAIL_HANLDER               (STL_SafetyFailure)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

const stc_stl_case_startup_t m_stcStartupCaseTable[] = {
    /* User implement the watchdog test cases. */
    STL_STARTUP_CASE(Watchdog, STL_WdtStartupTest, FAIL_HANLDER),

    /* STL implement the below test cases */
    STL_STARTUP_CASE(CPU,      STL_CpuTestStartup, FAIL_HANLDER),
    STL_STARTUP_CASE(PC,       STL_PcTest, FAIL_HANLDER),
    STL_STARTUP_CASE(Flash,    STL_FlashStartupTest, FAIL_HANLDER),
};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Power on self-test.
 * @param  None
 * @retval None
 */
void STL_StartupTest(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_ALL);

#if (STL_PRINT_ENABLE == STL_ON)
    (void)STL_PrintfInit();   /* startup debug print */
#endif

    STL_Printf("********   Self-test startup start   ********\r\n");

    STL_StartupTestCase(m_stcStartupCaseTable, ARRAY_SZ(m_stcStartupCaseTable));

    /* Note: Full ram test MUST be independently tested,  because test destroy stack. */
    if (STL_FullRamTestStartup(STL_RAM1_START, STL_RAM1_END) != STL_OK) {
        STL_Printf("********     RAM1 test fail in startup    ********\r\n");
        STL_SafetyFailure();
    }

    if (STL_FullRamTestStartup(STL_RAM2_START, STL_RAM2_END) != STL_OK) {
        STL_Printf("********     RAM2 test fail in startup    ********\r\n");
        STL_SafetyFailure();
    }

    STL_Printf("********      Self-test startup end      ********\r\n");

    /* Jump to the application after power-onself-test complete succesfully */
    CallApplicationStartUp();
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
