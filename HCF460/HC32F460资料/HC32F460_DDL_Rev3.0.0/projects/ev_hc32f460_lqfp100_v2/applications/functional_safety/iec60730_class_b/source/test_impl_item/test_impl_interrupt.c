/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/test_impl_item/interrupt_test.c
 * @brief This file provides firmware functions to manage the interrupt test.
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
#include "stl_test_interrupt.h"
#include "test_impl_interrupt.h"

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @defgroup Test_Implement_Interrupt Test Implement Interrupt
 * @{
 */

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup Test_Implement_Interrupt_Local_Macros Test Implement Interrupt Local Macros
 * @{
 */
#define TMR_VALUE                       ((SystemCoreClock/256UL)/2UL)
/**
 * @}
 */

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
/**
 * @defgroup Test_Implement_Interrupt_Local_Variables Test Implement Interrupt Local Variables
 * @{
 */
static stc_stl_int_params_t m_astcIntParamsTable[] = {
    {STL_TMRx_FREQ, STL_TMRx_FREQ - STL_TMRx_FREQ_OFFSET, STL_TMRx_FREQ + STL_TMRx_FREQ_OFFSET, 0U},
    {STL_TMRy_FREQ, STL_TMRy_FREQ - STL_TMRy_FREQ_OFFSET, STL_TMRy_FREQ + STL_TMRy_FREQ_OFFSET, 0U},
    {STL_TMRz_FREQ, STL_TMRz_FREQ - STL_TMRz_FREQ_OFFSET, STL_TMRz_FREQ + STL_TMRz_FREQ_OFFSET, 0U},
    {STL_TMRw_FREQ, STL_TMRw_FREQ - STL_TMRw_FREQ_OFFSET, STL_TMRw_FREQ + STL_TMRw_FREQ_OFFSET, 0U},
};
/**
 * @}
 */

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @defgroup Test_Implement_Interrupt_Global_Functions Test Implement Interrupt Global Functions
 * @{
 */

/**
 * @brief  TMRA1 interrupt callback.
 * @param  None
 * @retval None
 */
static void TMRx_OVF_IrqCallback(void)
{
    STL_IntUpdateCount(0U);
    TMRA_ClearStatus(STL_TMRx_UNIT, STL_TMRx_FLAG_OVF);
}

/**
 * @brief  TMRA2 interrupt callback.
 * @param  None
 * @retval None
 */
static void TMRy_OVF_IrqCallback(void)
{
    STL_IntUpdateCount(1U);
    TMRA_ClearStatus(STL_TMRy_UNIT, STL_TMRy_FLAG_OVF);
}

/**
 * @brief  TMRA3 interrupt callback.
 * @param  None
 * @retval None
 */
static void TMRz_OVF_IrqCallback(void)
{
    STL_IntUpdateCount(2U);
    TMRA_ClearStatus(STL_TMRz_UNIT, STL_TMRz_FLAG_OVF);
}

/**
 * @brief  TMRA4 interrupt callback.
 * @param  None
 * @retval None
 */
static void TMRw_OVF_IrqCallback(void)
{
    STL_IntUpdateCount(3U);
    TMRA_ClearStatus(STL_TMRw_UNIT, STL_TMRw_FLAG_OVF);
}

/**
 * @brief  Interrupt test initialize.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Initialize successfully.
 *           - STL_ERR:         Initialize unsuccessfully.
 */
uint32_t STL_IntRuntimeInit(void)
{
    stc_tmra_init_t  stcTimerInit;
    stc_irq_signin_config_t stcIrqRegiConf;

    STL_TMR_FCG_ENABLE();

    (void)TMRA_StructInit(&stcTimerInit);
    stcTimerInit.sw_count.u16ClockDiv = TMRA_CLK_DIV256;
    stcTimerInit.sw_count.u16CountDir  = TMRA_DIR_UP;
    stcTimerInit.sw_count.u16CountMode = TMRA_MD_SAWTOOTH;

    stcTimerInit.u32PeriodValue = (TMR_VALUE / STL_TMRx_FREQ);
    (void)TMRA_Init(STL_TMRx_UNIT, &stcTimerInit);

    stcTimerInit.u32PeriodValue = (TMR_VALUE / STL_TMRy_FREQ);
    (void)TMRA_Init(STL_TMRy_UNIT, &stcTimerInit);

    stcTimerInit.u32PeriodValue = (TMR_VALUE / STL_TMRz_FREQ);
    (void)TMRA_Init(STL_TMRz_UNIT, &stcTimerInit);

    stcTimerInit.u32PeriodValue = (TMR_VALUE / STL_TMRw_FREQ);
    (void)TMRA_Init(STL_TMRw_UNIT, &stcTimerInit);

    TMRA_IntCmd(STL_TMRx_UNIT, STL_TMRx_INT_OVF, ENABLE);
    stcIrqRegiConf.enIntSrc = STL_TMRx_OVF_INT_SRC;
    stcIrqRegiConf.enIRQn = STL_TMRx_OVF_INT_IRQn;
    stcIrqRegiConf.pfnCallback = TMRx_OVF_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, STL_TMRx_OVF_IRQ_PRIO);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    TMRA_IntCmd(STL_TMRy_UNIT, STL_TMRy_INT_OVF, ENABLE);
    stcIrqRegiConf.enIntSrc = STL_TMRy_OVF_INT_SRC;
    stcIrqRegiConf.enIRQn = STL_TMRy_OVF_INT_IRQn;
    stcIrqRegiConf.pfnCallback = TMRy_OVF_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, STL_TMRy_OVF_IRQ_PRIO);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    TMRA_IntCmd(STL_TMRz_UNIT, STL_TMRz_INT_OVF, ENABLE);
    stcIrqRegiConf.enIntSrc = STL_TMRz_OVF_INT_SRC;
    stcIrqRegiConf.enIRQn = STL_TMRz_OVF_INT_IRQn;
    stcIrqRegiConf.pfnCallback = TMRz_OVF_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, STL_TMRz_OVF_IRQ_PRIO);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    TMRA_IntCmd(STL_TMRw_UNIT, STL_TMRw_INT_OVF, ENABLE);
    stcIrqRegiConf.enIntSrc = STL_TMRw_OVF_INT_SRC;
    stcIrqRegiConf.enIRQn = STL_TMRw_OVF_INT_IRQn;
    stcIrqRegiConf.pfnCallback = TMRw_OVF_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, STL_TMRw_OVF_IRQ_PRIO);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    (void)STL_IntRuntimeTableInit(m_astcIntParamsTable, ARRAY_SZ(m_astcIntParamsTable));

    TMRA_Start(STL_TMRx_UNIT);
    TMRA_Start(STL_TMRy_UNIT);
    TMRA_Start(STL_TMRz_UNIT);
    TMRA_Start(STL_TMRw_UNIT);

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
