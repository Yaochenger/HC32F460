/**
 *******************************************************************************
 * @file  timer4/timer4_oc_double_ch/source/main.c
 * @brief This example demonstrates how to use the link double channel of TMR4
 *        output-compare function.
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
 * @addtogroup TIMER4_OC_Double_Channels
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* TMR4 unit definition */
#define TMR4_UNIT                       (CM_TMR4_1)
#define TMR4_FCG_ENABLE()               (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR4_1, ENABLE))

/* TMR4 count period value(500ms) */
#define TMR4_CNT_PERIOD_VALUE(div)      ((uint16_t)(TMR4_ClockFreq() / (1UL << (uint32_t)(div)) / 2UL) - 1U)

/* TMR4 output-compare channel definition */
#define TMR4_OC_CH_XH                   (TMR4_OC_CH_UH)
#define TMR4_OC_CH_XL                   (TMR4_OC_CH_XH + 1UL)

/* TMR4 output-compare flag and interupt definition */
#define TMR4_FLAG_OC_CMP                (TMR4_FLAG_OC_CMP_UH | TMR4_FLAG_OC_CMP_UL)
#define TMR4_INT_OC_CMP                 (TMR4_INT_OC_CMP_UH | TMR4_INT_OC_CMP_UL)

/* TMR4 output-compare interrupt source definition */
#define TMR4_OC_HIGH_CH_IRQn            (INT000_IRQn)
#define TMR4_OC_HIGH_CH_INT_SRC         (INT_SRC_TMR4_1_GCMP_UH)

#define TMR4_OC_LOW_CH_IRQn             (INT001_IRQn)
#define TMR4_OC_LOW_CH_INT_SRC          (INT_SRC_TMR4_1_GCMP_UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static __IO en_functional_state_t m_enToggleLed = DISABLE;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Get TMR4 clock frequency.
 * @param  None
 * @retval TMR4 clock frequency
 */
static uint32_t TMR4_ClockFreq(void)
{
    return CLK_GetBusClockFreq(CLK_BUS_PCLK1);
}

/**
 * @brief  TMR4 output-compare match interrupt handler callback.
 * @param  None
 * @retval None
 */
static void TMR4_OC_IrqCallback(void)
{
    static uint16_t m_u16OcPolarity = TMR4_OC_INVD_LOW;
    const uint16_t u16CurrentPolarity = TMR4_OC_GetPolarity(TMR4_UNIT, TMR4_OC_CH_XL);

    if (m_u16OcPolarity != u16CurrentPolarity) {
        m_enToggleLed = ENABLE;
        m_u16OcPolarity = u16CurrentPolarity;
    }

    TMR4_ClearStatus(TMR4_UNIT, TMR4_FLAG_OC_CMP);
}

/**
 * @brief  Main function of TMR4 output-compare link double channels
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_irq_signin_config_t stcIrqConfig;
    stc_tmr4_init_t stcTmr4Init;
    stc_tmr4_oc_init_t stcTmr4OcInit;
    un_tmr4_oc_ocmrh_t unTmr4OcOcmrh;
    un_tmr4_oc_ocmrl_t unTmr4OcOcmrl;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable TMR4 peripheral clock */
    TMR4_FCG_ENABLE();

    /* Set PCLK1 */
    CLK_SetClockDiv(CLK_BUS_PCLK1, CLK_PCLK1_DIV2);

    /************************* Configure TMR4 counter *************************/
    stcTmr4Init.u16ClockSrc = TMR4_CLK_SRC_INTERNCLK;
    stcTmr4Init.u16ClockDiv = TMR4_CLK_DIV1024;
    stcTmr4Init.u16CountMode = TMR4_MD_SAWTOOTH;
    stcTmr4Init.u16PeriodValue = TMR4_CNT_PERIOD_VALUE(stcTmr4Init.u16ClockDiv);
    (void)TMR4_Init(TMR4_UNIT, &stcTmr4Init);

    /************************* Configure TMR4 output-compare ******************/
    /* Initialize TMR4 OC structure */
    (void)TMR4_OC_StructInit(&stcTmr4OcInit);

    /* TMR4 OC high/low channel: initialize */
    stcTmr4OcInit.u16CompareValue = stcTmr4Init.u16PeriodValue / 4U;
    (void)TMR4_OC_Init(TMR4_UNIT, TMR4_OC_CH_XH, &stcTmr4OcInit);

    stcTmr4OcInit.u16CompareValue = (stcTmr4Init.u16PeriodValue / 4U) * 3U;
    (void)TMR4_OC_Init(TMR4_UNIT, TMR4_OC_CH_XL, &stcTmr4OcInit);

    /* TMR4 OC high channel: compare mode OCMR[15:0] = 0x000F = b 0000 0000 0000 1111 */
    unTmr4OcOcmrh.OCMRx = 0U;
    unTmr4OcOcmrh.OCMRx_f.OCFDCH = TMR4_OC_OCF_SET; /* bit[0]      1  */
    unTmr4OcOcmrh.OCMRx_f.OCFPKH = TMR4_OC_OCF_SET; /* bit[1]      1  */
    unTmr4OcOcmrh.OCMRx_f.OCFUCH = TMR4_OC_OCF_SET; /* bit[2]      1  */
    unTmr4OcOcmrh.OCMRx_f.OCFZRH = TMR4_OC_OCF_SET; /* bit[3]      1  */
    unTmr4OcOcmrh.OCMRx_f.OPDCH  = TMR4_OC_HOLD;    /* Bit[5:4]    00 */
    unTmr4OcOcmrh.OCMRx_f.OPPKH  = TMR4_OC_HOLD;    /* Bit[7:6]    00 */
    unTmr4OcOcmrh.OCMRx_f.OPUCH  = TMR4_OC_HOLD;    /* Bit[9:8]    00 */
    unTmr4OcOcmrh.OCMRx_f.OPZRH  = TMR4_OC_HOLD;    /* Bit[11:10]  00 */
    unTmr4OcOcmrh.OCMRx_f.OPNPKH = TMR4_OC_HOLD;    /* Bit[13:12]  00 */
    unTmr4OcOcmrh.OCMRx_f.OPNZRH = TMR4_OC_HOLD;    /* Bit[15:14]  00 */
    TMR4_OC_SetHighChCompareMode(TMR4_UNIT, TMR4_OC_CH_XH, unTmr4OcOcmrh);

    /* TMR4 OC low channel: compare mode OCMR[31:0] Ox FFFF 0FFF = b 1111 1111 1111 1111   0000 1111 1111 1111 */
    unTmr4OcOcmrl.OCMRx = 0UL;
    unTmr4OcOcmrl.OCMRx_f.OCFDCL  = TMR4_OC_OCF_SET; /* bit[0]      1  */
    unTmr4OcOcmrl.OCMRx_f.OCFPKL  = TMR4_OC_OCF_SET; /* bit[1]      1  */
    unTmr4OcOcmrl.OCMRx_f.OCFUCL  = TMR4_OC_OCF_SET; /* bit[2]      1  */
    unTmr4OcOcmrl.OCMRx_f.OCFZRL  = TMR4_OC_OCF_SET; /* bit[3]      1  */
    unTmr4OcOcmrl.OCMRx_f.OPDCL   = TMR4_OC_INVT;    /* bit[5:4]    11 */
    unTmr4OcOcmrl.OCMRx_f.OPPKL   = TMR4_OC_INVT;    /* bit[7:6]    11 */
    unTmr4OcOcmrl.OCMRx_f.OPUCL   = TMR4_OC_INVT;    /* bit[9:8]    11 */
    unTmr4OcOcmrl.OCMRx_f.OPZRL   = TMR4_OC_INVT;    /* bit[11:10]  11 */
    unTmr4OcOcmrl.OCMRx_f.OPNPKL  = TMR4_OC_HOLD;    /* bit[13:12]  00 */
    unTmr4OcOcmrl.OCMRx_f.OPNZRL  = TMR4_OC_HOLD;    /* bit[15:14]  00 */
    unTmr4OcOcmrl.OCMRx_f.EOPNDCL = TMR4_OC_INVT;    /* bit[17:16]  00 */
    unTmr4OcOcmrl.OCMRx_f.EOPNUCL = TMR4_OC_INVT;    /* bit[19:18]  00 */
    unTmr4OcOcmrl.OCMRx_f.EOPDCL  = TMR4_OC_INVT;    /* bit[21:20]  11 */
    unTmr4OcOcmrl.OCMRx_f.EOPPKL  = TMR4_OC_INVT;    /* bit[23:22]  11 */
    unTmr4OcOcmrl.OCMRx_f.EOPUCL  = TMR4_OC_INVT;    /* bit[25:24]  11 */
    unTmr4OcOcmrl.OCMRx_f.EOPZRL  = TMR4_OC_INVT;    /* bit[27:26]  11 */
    unTmr4OcOcmrl.OCMRx_f.EOPNPKL = TMR4_OC_INVT;    /* bit[29:28]  11 */
    unTmr4OcOcmrl.OCMRx_f.EOPNZRL = TMR4_OC_INVT;    /* bit[31:30]  11 */
    TMR4_OC_SetLowChCompareMode(TMR4_UNIT, TMR4_OC_CH_XL, unTmr4OcOcmrl);

    /* TMR4 OC high/low channel: register IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = TMR4_OC_HIGH_CH_IRQn;
    stcIrqConfig.enIntSrc = TMR4_OC_HIGH_CH_INT_SRC;
    stcIrqConfig.pfnCallback = &TMR4_OC_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqConfig);
    NVIC_ClearPendingIRQ(stcIrqConfig.enIRQn);
    NVIC_SetPriority(stcIrqConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqConfig.enIRQn);

    stcIrqConfig.enIRQn = TMR4_OC_LOW_CH_IRQn;
    stcIrqConfig.enIntSrc = TMR4_OC_LOW_CH_INT_SRC;
    stcIrqConfig.pfnCallback = &TMR4_OC_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqConfig);
    NVIC_ClearPendingIRQ(stcIrqConfig.enIRQn);
    NVIC_SetPriority(stcIrqConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqConfig.enIRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* TMR4 OC high/low channel: enable interrupt */
    TMR4_IntCmd(TMR4_UNIT, TMR4_INT_OC_CMP, ENABLE);

    /* TMR4 OC high/low channel: enable output-compare */
    TMR4_OC_Cmd(TMR4_UNIT, TMR4_OC_CH_XH, ENABLE);
    TMR4_OC_Cmd(TMR4_UNIT, TMR4_OC_CH_XL, ENABLE);

    /* Start TMR4 count. */
    TMR4_Start(TMR4_UNIT);

    for (;;) {
        if (ENABLE == m_enToggleLed) {
            m_enToggleLed = DISABLE;
            BSP_LED_Toggle(LED_BLUE);
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
