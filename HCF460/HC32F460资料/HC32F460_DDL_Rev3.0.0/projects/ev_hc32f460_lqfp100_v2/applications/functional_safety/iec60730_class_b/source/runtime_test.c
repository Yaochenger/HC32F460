/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/runtime_test.c
 * @brief IEC60730 class B for the runtime test.
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
#include "stl_bsp_conf.h"
#include "stl_utility.h"
#include "stl_test_cpu.h"
#include "stl_test_flash.h"
#include "stl_test_interrupt.h"
#include "stl_test_pc.h"
#include "stl_test_ram.h"
#include "stl_test_runtime.h"
#include "runtime_test.h"
#include "test_impl_adc.h"
#include "test_impl_clk.h"
#include "test_impl_gpio.h"
#include "test_impl_interrupt.h"
#include "test_impl_wdt.h"

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @addtogroup Runtime_Test
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
volatile uint32_t g_u32SysTickCount;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

static const stc_stl_case_runtime_t m_stcRuntimeCaseTable[] = {
    /* STL implement the below test cases */
    STL_RUNTIME_CASE(CPU,   NULL,                 STL_CpuTestRuntime,   NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(Flash, NULL,                 STL_FlashRuntimeTest, NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(PC,    NULL,                 STL_PcTest, NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(RAM,   STL_RamRuntimeInit,   STL_RamRuntimeTest, NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(Stack, STL_StackRuntimeInit, STL_StackRuntimeTest, NULL, FAIL_HANLDER),

    /* User implement the below test cases. */
    STL_RUNTIME_CASE(ADC,         STL_AdcRuntimeInit,        STL_AdcRuntimeTest,        NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(GPIO Input,  STL_GpioInputRuntimeInit,  STL_GpioInputRuntimeTest,  NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(GPIO Output, STL_GpioOutputRuntimeInit, STL_GpioOutputRuntimeTest, NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(Watchdog,    STL_WdtRuntimeInit,        STL_WdtRuntimeFeed,        NULL, FAIL_HANLDER),
};

static const stc_stl_case_runtime_t m_stcPeriodCaseTable[] = {
    /* User implement the below test cases. */
    STL_RUNTIME_CASE(Clock,     STL_ClkRuntimeInit, STL_ClkRuntimeTest, NULL, FAIL_HANLDER),
    STL_RUNTIME_CASE(Interrupt, STL_IntRuntimeInit, STL_IntRuntimeTest, NULL, FAIL_HANLDER),
};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Get runtime case table.
 * @param  None
 * @retval Pointer to runtime case table.
 */
const stc_stl_case_runtime_t *TEST_GetRuntimeCaseTable(void)
{
    return m_stcRuntimeCaseTable;
}

/**
 * @brief  Get runtime case table size.
 * @param  None
 * @retval Runtime case table size.
 */
uint32_t TEST_GetRuntimeCaseTableSize(void)
{
    return ARRAY_SZ(m_stcRuntimeCaseTable);
}

/**
 * @brief  Get runtime case table.
 * @param  None
 * @retval Pointer to runtime case table.
 */
const stc_stl_case_runtime_t *TEST_GetPeriodCaseTable(void)
{
    return m_stcPeriodCaseTable;
}

/**
 * @brief  Get runtime case table size.
 * @param  None
 * @retval Runtime case table size.
 */
uint32_t TEST_GetPeriodCaseTableSize(void)
{
    return ARRAY_SZ(m_stcPeriodCaseTable);
}

/**
 * @brief  Test feed watchdog.
 * @param  None
 * @retval None
 */
void TEST_FeedWatchdog(void)
{
    (void)STL_WdtRuntimeFeed();
}

/**
 * @brief  SysTick interrupt callback function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    g_u32SysTickCount++;
    STL_RuntimeTestCase(m_stcPeriodCaseTable,  ARRAY_SZ(m_stcPeriodCaseTable));
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
