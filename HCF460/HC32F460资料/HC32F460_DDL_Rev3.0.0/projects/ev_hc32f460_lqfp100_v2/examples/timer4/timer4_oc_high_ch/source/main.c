/**
 *******************************************************************************
 * @file  timer4/timer4_oc_high_ch/source/main.c
 * @brief This example demonstrates how to use the single high channel of TMR4
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
 * @addtogroup TIMER4_OC_High_Channel
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
#define TMR4_OC_CH                      (TMR4_OC_CH_UH)

/* TMR4 output-compare flag and interupt definition */
#define TMR4_FLAG_OC_CMP                (TMR4_FLAG_OC_CMP_UH)
#define TMR4_INT_OC_CMP                 (TMR4_INT_OC_CMP_UH)

/* TMR4 output-compare interrupt source definition */
#define TMR4_OC_IRQn                    (INT000_IRQn)
#define TMR4_OC_INT_SRC                 (INT_SRC_TMR4_1_GCMP_UH)

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
    m_enToggleLed = ENABLE;
    TMR4_ClearStatus(TMR4_UNIT, TMR4_FLAG_OC_CMP);
}

/**
 * @brief  Main function of TMR4 output-compare single high channel
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_irq_signin_config_t stcIrqConfig;
    stc_tmr4_init_t stcTmr4Init;
    stc_tmr4_oc_init_t stcTmr4OcInit;
    un_tmr4_oc_ocmrh_t unTmr4OcOcmrh;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable TMR4 peripheral clock */
    TMR4_FCG_ENABLE();

    /* Set PCLK0 */
    CLK_SetClockDiv(CLK_BUS_PCLK1, CLK_PCLK1_DIV2);

    /************************* Configure TMR4 counter *************************/
    stcTmr4Init.u16ClockSrc = TMR4_CLK_SRC_INTERNCLK;
    stcTmr4Init.u16ClockDiv = TMR4_CLK_DIV1024;
    stcTmr4Init.u16CountMode = TMR4_MD_SAWTOOTH;
    stcTmr4Init.u16PeriodValue = TMR4_CNT_PERIOD_VALUE(stcTmr4Init.u16ClockDiv);
    (void)TMR4_Init(TMR4_UNIT, &stcTmr4Init);

    /************************* Configure TMR4 output-compare ******************/
    /* TMR4 OC high channel: initialize */
    (void)TMR4_OC_StructInit(&stcTmr4OcInit);
    stcTmr4OcInit.u16CompareValue = stcTmr4Init.u16PeriodValue / 2U;
    (void)TMR4_OC_Init(TMR4_UNIT, TMR4_OC_CH, &stcTmr4OcInit);

    /* TMR4 OC high channel: compare mode OCMR[15:0] = 0x0FFF = b 0000 0000 0000 0100 */
    unTmr4OcOcmrh.OCMRx = 0U;
    unTmr4OcOcmrh.OCMRx_f.OCFUCH = TMR4_OC_OCF_SET; /* bit[2] 1  */
    TMR4_OC_SetHighChCompareMode(TMR4_UNIT, TMR4_OC_CH, unTmr4OcOcmrh);

    /* TMR4 OC high channel: register IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = TMR4_OC_IRQn;
    stcIrqConfig.enIntSrc = TMR4_OC_INT_SRC;
    stcIrqConfig.pfnCallback = &TMR4_OC_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqConfig);
    NVIC_ClearPendingIRQ(stcIrqConfig.enIRQn);
    NVIC_SetPriority(stcIrqConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqConfig.enIRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* TMR4 OC high channel: enable interrupt */
    TMR4_IntCmd(TMR4_UNIT, TMR4_INT_OC_CMP, ENABLE);

    /* TMR4 OC high channel: enable output-compare */
    TMR4_OC_Cmd(TMR4_UNIT, TMR4_OC_CH, ENABLE);

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
