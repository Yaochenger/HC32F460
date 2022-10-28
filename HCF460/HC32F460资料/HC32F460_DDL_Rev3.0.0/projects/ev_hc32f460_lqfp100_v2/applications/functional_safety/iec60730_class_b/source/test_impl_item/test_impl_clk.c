/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/test_impl_clk.c
 * @brief This file provides firmware functions to implement the clock test.
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
#include "stl_bsp_conf.h"
#include "test_impl_clk.h"

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @defgroup Test_Implement_Clock Test Implement Clock
 * @{
 */

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define REF_CLK_FREQ                (XTAL32_VALUE)
#define REF_CLK_DIV                 (32UL)

#define TARGET_CLK_DIV              (32UL)
#define TARGET_CLK_FREQ             (SystemCoreClock)

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
static __IO uint32_t m_u32ClockErrCount = 0UL;
static __IO uint8_t m_u32FmcActived = STL_OFF;

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @defgroup Test_Implement_Clock_Global_Functions Test Implement Clock Global Functions
 * @{
 */

/**
 * @brief  FCM reference clock XTAL32 initialize
 * @param  None
 * @retval None
 */
static void RefClockInit(void)
{
    stc_clock_xtal32_init_t stcXtal32Init;

    /* Xtal32 config */
    (void)CLK_Xtal32StructInit(&stcXtal32Init);
    stcXtal32Init.u8State = CLK_XTAL32_ON;
    stcXtal32Init.u8Drv   = CLK_XTAL32_DRV_MID;
    stcXtal32Init.u8Filter = CLK_XTAL32_FILTER_ALL_MD;
    (void)CLK_Xtal32Init(&stcXtal32Init);
}

/**
 * @brief  FCM frequency error IRQ callback
 * @param  None
 * @retval None
 */
static void FCM_Error_IrqCallback(void)
{
    FCM_Cmd(DISABLE);

    m_u32ClockErrCount++;
    m_u32FmcActived = STL_OFF;

    FCM_ClearStatus(FCM_FLAG_ERR);
}

/**
 * @brief  FCM measure counter overflow IRQ callback
 * @param  None
 * @retval None
 */
static void FCM_Ovf_IrqCallback(void)
{
    FCM_Cmd(DISABLE);

    m_u32ClockErrCount++;
    m_u32FmcActived = STL_OFF;

    FCM_ClearStatus(FCM_FLAG_OVF);
}

/**
 * @brief  Clock test initialize in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Initialize successfully.
 *           - STL_ERR:         Initialize unsuccessfully.
 */
uint32_t STL_ClkRuntimeInit(void)
{
    stc_fcm_init_t stcFcmInit;
    stc_irq_signin_config_t stcIrqSignConfig;

    RefClockInit();

    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_FCM, ENABLE);

    (void)FCM_StructInit(&stcFcmInit);
    stcFcmInit.u32RefClock     = FCM_REF_CLK_XTAL32;
    stcFcmInit.u32RefClockDiv  = FCM_REF_CLK_DIV32;
    stcFcmInit.u32RefClockEdge = FCM_REF_CLK_RISING;
    stcFcmInit.u32TargetClock  = FCM_TARGET_CLK_MPLLP;
    stcFcmInit.u32ExceptionType = FCM_EXP_TYPE_INT;
    stcFcmInit.u32TargetClockDiv = FCM_TARGET_CLK_DIV32;

    /* Idea count value = (targ_freq/tar_div)/(ref_freq/ref_div) */
    stcFcmInit.u16LowerLimit = (uint16_t)((((TARGET_CLK_FREQ / TARGET_CLK_DIV) / (REF_CLK_FREQ / REF_CLK_DIV)) * 97UL) / 100UL);
    stcFcmInit.u16UpperLimit = (uint16_t)((((TARGET_CLK_FREQ / TARGET_CLK_DIV) / (REF_CLK_FREQ / REF_CLK_DIV)) * 103UL) / 100UL);

    (void)FCM_Init(&stcFcmInit);
    FCM_IntCmd((FCM_INT_OVF | FCM_INT_ERR), ENABLE);

    stcIrqSignConfig.enIntSrc = INT_SRC_FCMFERRI;
    stcIrqSignConfig.enIRQn   = STL_FCM_ERR_INT_IRQn;
    stcIrqSignConfig.pfnCallback = &FCM_Error_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, STL_FCM_ERR_IRQ_PRIO);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    stcIrqSignConfig.enIntSrc = INT_SRC_FCMCOVFI;
    stcIrqSignConfig.enIRQn   = STL_FCM_OVF_INT_IRQn;
    stcIrqSignConfig.pfnCallback = &FCM_Ovf_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, STL_FCM_OVF_IRQ_PRIO);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    return STL_OK;
}

/**
 * @brief  Clock test in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Test pass.
 *           - STL_ERR:         Test fail.
 */
uint32_t STL_ClkRuntimeTest(void)
{
    if (STL_OFF == m_u32FmcActived) {
        m_u32ClockErrCount = 0UL;
        m_u32FmcActived = STL_ON;

        FCM_Cmd(ENABLE);
    }

    return (m_u32ClockErrCount == 0UL) ? STL_OK : STL_ERR;
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
