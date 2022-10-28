/**
 *******************************************************************************
 * @file  timer4/timer4_counter_sawtooth/source/main.c
 * @brief This example demonstrates TMR4 counter sawtooth wave mode.
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
 * @addtogroup TIMER4_Counter_Sawtooth
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                       (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                             LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* TMR4 unit definition */
#define TMR4_UNIT                           (CM_TMR4_1)
#define TMR4_FCG_ENABLE()                   (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR4_1, ENABLE))

/* TMR4 count value per second */
#define TMR4_COUNT_VALUE_PER_SECOND(div)    ((TMR4_ClockFreq() / (1UL << (uint32_t)(div))))

/* TMR4 interrupt definition */
#define TMR4_COUNT_VALLEY_IRQn              (INT000_IRQn)
#define TMR4_COUNT_VALLEY_INT_SRC           (INT_SRC_TMR4_1_UDF)

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
 * @brief  TMR4 Count valley interrupt handler callback.
 * @param  None
 * @retval None
 */
static void TMR4_CountValley_IrqCallback(void)
{
    m_enToggleLed = ENABLE;
    TMR4_ClearStatus(TMR4_UNIT, TMR4_FLAG_CNT_VALLEY);
}

/**
 * @brief  Main function of TMR4 counter sawtooth wave mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_tmr4_init_t stcTmr4Init;
    stc_irq_signin_config_t stcIrqConfig;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable TMR4 peripheral clock */
    TMR4_FCG_ENABLE();

    /* Initialize TMR4 Counter */
    stcTmr4Init.u16CountMode = TMR4_MD_SAWTOOTH;
    stcTmr4Init.u16ClockSrc = TMR4_CLK_SRC_INTERNCLK;
    stcTmr4Init.u16ClockDiv = TMR4_CLK_DIV64;
    stcTmr4Init.u16PeriodValue = (uint16_t)(TMR4_COUNT_VALUE_PER_SECOND(stcTmr4Init.u16ClockDiv) / \
                                            (2UL * ((uint32_t)TMR4_INT_CNT_MASK15 + 1UL))) - 1U;
    (void)TMR4_Init(TMR4_UNIT, &stcTmr4Init);

    /* Set TMR4 counter interrupt mask times */
    TMR4_SetCountIntMaskTime(TMR4_UNIT, TMR4_INT_CNT_VALLEY, TMR4_INT_CNT_MASK15);

    /* Register IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = TMR4_COUNT_VALLEY_IRQn;
    stcIrqConfig.enIntSrc = TMR4_COUNT_VALLEY_INT_SRC;
    stcIrqConfig.pfnCallback = &TMR4_CountValley_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqConfig);
    NVIC_ClearPendingIRQ(stcIrqConfig.enIRQn);
    NVIC_SetPriority(stcIrqConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqConfig.enIRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Enable the valley interrupt */
    TMR4_IntCmd(TMR4_UNIT, TMR4_INT_CNT_VALLEY, ENABLE);

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
