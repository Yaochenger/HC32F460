/**
 *******************************************************************************
 * @file  timera/timera_compare_value_buffer/source/main.c
 * @brief Main program TimerA compare value buffer for the Device Driver Library.
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
 * @addtogroup TIMERA_Compare_Value_Buffer
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*
 * TimerA unit and channel definitions for this example.
 *    |----------------|----------------|
 *    |  TMRA_BUF_CH   |   TMRA_DEST_CH |
 *    |----------------|----------------|
 *    |  TMRA_CH2      |   TMRA_CH1     |
 *    |----------------|----------------|
 *    |  TMRA_CH4      |   TMRA_CH3     |
 *    |----------------|----------------|
 *    |  TMRA_CH6      |   TMRA_CH5     |
 *    |----------------|----------------|
 *    |  TMRA_CH8      |   TMRA_CH7     |
 *    |----------------|----------------|
 */
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_BUF_CH                     (TMRA_CH6)
#define TMRA_DEST_CH                    (TMRA_CH5)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)

/* The counting mode of TimerA. @ref TMRA_Count_Mode */
#define TMRA_MD                         (TMRA_MD_TRIANGLE)

#if (TMRA_MD == TMRA_MD_SAWTOOTH)
/* The counting direction of TimerA. @ref TMRA_Count_Dir */
#define TMRA_DIR                        (TMRA_DIR_DOWN)
#endif

/* The divider of the clock source. @ref TMRA_Clock_Divider */
#define TMRA_CLK_DIV                    (TMRA_CLK_DIV1024)

/* Period value and compare value. */
#define TMRA_PERIOD_VAL                 (0x2000U)
#define TMRA_SRC_CMP_VAL                (0x1000U)

/*
 * Compare value buffer condition.
 * 'TMRA_CMP_BUF_TRANS_COND' can be defined as a value of @ref TMRA_Cmp_Value_Buf_Trans_Cond
 */
#if (TMRA_MD == TMRA_MD_TRIANGLE)
/* 'TMRA_CMP_BUF_TRANS_COND' can be defined as 'TMRA_BUF_TRANS_COND_PEAK' or 'TMRA_BUF_TRANS_COND_VALLEY' or both. */
#define TMRA_CMP_BUF_TRANS_COND         (TMRA_BUF_TRANS_COND_PEAK | TMRA_BUF_TRANS_COND_VALLEY)
#else
#define TMRA_CMP_BUF_TRANS_COND         (TMRA_BUF_TRANS_COND_OVF_UDF_CLR)
#endif

/* Definitions about TimerA interrupt for the example. */
#define TMRA_INT_OVF_TYPE               (TMRA_INT_OVF)
#define TMRA_INT_OVF_FLAG               (TMRA_FLAG_OVF)
#define TMRA_INT_OVF_PRIO               (DDL_IRQ_PRIO_03)
#define TMRA_INT_OVF_SRC                (INT_SRC_TMRA_1_OVF)
#define TMRA_INT_OVF_IRQn               (INT080_IRQn)

#define TMRA_INT_UDF_TYPE               (TMRA_INT_UDF)
#define TMRA_INT_UDF_FLAG               (TMRA_FLAG_UDF)
#define TMRA_INT_UDF_PRIO               (DDL_IRQ_PRIO_03)
#define TMRA_INT_UDF_SRC                (INT_SRC_TMRA_1_UDF)
#define TMRA_INT_UDF_IRQn               (INT081_IRQn)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void TmrAConfig(void);
static void TmrAIrqConfig(void);

static void TMRA_Ovf_IrqCallback(void);
static void TMRA_Udf_IrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
__IO static uint16_t m_u16CompareValueBuffer = TMRA_SRC_CMP_VAL;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timera_compare_value_buffer project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Initializes UART for debug printing. Baudrate is 19200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 19200U, BSP_PRINTF_Preinit);
    /* Configures TimerA. */
    TmrAConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    /* Starts TimerA. */
    TMRA_Start(TMRA_UNIT);

    /***************** Configuration end, application start **************/
    for (;;) {
        /* See TMRA_Ovf_IrqCallback in this file. */
    }
}

/**
 * @brief  TimerA configuration.
 * @param  None
 * @retval None
 */
static void TmrAConfig(void)
{
    stc_tmra_init_t stcTmraInit;

    /* 1. Enable TimerA peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMRA_PERIPH_CLK, ENABLE);

    /* 2. Initialize TimerA low unit . */
    (void)TMRA_StructInit(&stcTmraInit);
    stcTmraInit.sw_count.u16ClockDiv  = TMRA_CLK_DIV;
    stcTmraInit.sw_count.u16CountMode = TMRA_MD;
#if (TMRA_MD == TMRA_MD_SAWTOOTH)
    stcTmraInit.sw_count.u16CountDir  = TMRA_DIR;
#endif
    stcTmraInit.u32PeriodValue = TMRA_PERIOD_VAL;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);
    TMRA_SetCompareValue(TMRA_UNIT, TMRA_BUF_CH, TMRA_SRC_CMP_VAL);

    /* 3. Condition of compare value buffer transmission. */
    TMRA_SetCompareBufCond(TMRA_UNIT, TMRA_DEST_CH, TMRA_CMP_BUF_TRANS_COND);
    TMRA_CompareBufCmd(TMRA_UNIT, TMRA_DEST_CH, ENABLE);

    /* 4. Configures IRQ if needed. */
    TmrAIrqConfig();
}

/**
 * @brief  TimerA interrupt configuration.
 * @param  None
 * @retval None
 */
static void TmrAIrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = TMRA_INT_OVF_SRC;
    stcIrq.enIRQn      = TMRA_INT_OVF_IRQn;
    stcIrq.pfnCallback = &TMRA_Ovf_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, TMRA_INT_OVF_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    stcIrq.enIntSrc    = TMRA_INT_UDF_SRC;
    stcIrq.enIRQn      = TMRA_INT_UDF_IRQn;
    stcIrq.pfnCallback = &TMRA_Udf_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, TMRA_INT_UDF_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    /* Enable the specified interrupts of TimerA. */
    TMRA_IntCmd(TMRA_UNIT, TMRA_INT_OVF_TYPE | TMRA_INT_UDF_TYPE, ENABLE);
}

/**
 * @brief  TimerA counter overflow interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMRA_Ovf_IrqCallback(void)
{
    TMRA_ClearStatus(TMRA_UNIT, TMRA_INT_OVF_FLAG);
    DDL_Printf("OVF Irq callback get current compare value: %u\r\n",
               (unsigned int)TMRA_GetCompareValue(TMRA_UNIT, TMRA_DEST_CH));
    m_u16CompareValueBuffer++;
    TMRA_SetCompareValue(TMRA_UNIT, TMRA_BUF_CH, m_u16CompareValueBuffer);
    DDL_Printf("OVF Irq callback set compare value buffer: %u\r\n", m_u16CompareValueBuffer);
}

/**
 * @brief  TimerA counter underflow interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMRA_Udf_IrqCallback(void)
{
    TMRA_ClearStatus(TMRA_UNIT, TMRA_INT_UDF_FLAG);
    DDL_Printf("UDF Irq callback get current compare value: %u\r\n",
               (unsigned int)TMRA_GetCompareValue(TMRA_UNIT, TMRA_DEST_CH));
    m_u16CompareValueBuffer++;
    TMRA_SetCompareValue(TMRA_UNIT, TMRA_BUF_CH, m_u16CompareValueBuffer);
    DDL_Printf("UDF Irq callback set compare value buffer: %u\r\n", m_u16CompareValueBuffer);
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
