/**
 *******************************************************************************
 * @file  timer4/timer4_pwm_reload_int/source/main.c
 * @brief This example demonstrates how to use the reload-timer of TMR4 PWM.
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
 * @addtogroup TIMER4_PWM_Reload_Int
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

/* TMR4 PWM channel definition */
#define TMR4_PWM_CH                     (TMR4_PWM_CH_U)

/* TMR4 PWM flag and interupt definition */
#define TMR4_FLAG_RELOAD_TMR            (TMR4_FLAG_RELOAD_TMR_U)
#define TMR4_INT_RELOAD_TMR             (TMR4_INT_RELOAD_TMR_U)

/* TMR4 PWM reload-timer period value(31.25ms) */
#define TMR4_PWM_RT_VALUE(div)          ((uint16_t)(TMR4_ClockFreq() / (1UL << (uint32_t)(div)) / 32UL))

/* TMR4 PWM reload-timer interrupt definition */
#define TMR4_PWM_RT_IRQn                (INT000_IRQn)
#define TMR4_PWM_RT_INT_SRC             (INT_SRC_TMR4_1_RELOAD_U)

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
 * @brief  TMR4 PWM reload timer interrupt handler callback.
 * @param  None
 * @retval None
 */
static void TMR4_PWMReloadTimer_IrqCallback(void)
{
    static uint8_t u8IrqCount = 0U;

    if (++u8IrqCount >= 16U) {
        u8IrqCount = 0U;
        m_enToggleLed = ENABLE;
    }

    TMR4_ClearStatus(TMR4_UNIT, TMR4_FLAG_RELOAD_TMR);
}

/**
 * @brief  Main function of TMR4 PWM reload-timer mode
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_tmr4_pwm_init_t stcTmr4PwmInit;
    stc_irq_signin_config_t stcIrqConfig;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable TMR4 peripheral clock */
    TMR4_FCG_ENABLE();

    /************************* Configure TMR4 PWM *************************/
    (void)TMR4_PWM_StructInit(&stcTmr4PwmInit);
    stcTmr4PwmInit.u16ClockDiv = TMR4_PWM_CLK_DIV128;
    stcTmr4PwmInit.u16Mode = TMR4_PWM_MD_DEAD_TMR;
    (void)TMR4_PWM_Init(TMR4_UNIT, TMR4_PWM_CH, &stcTmr4PwmInit);

    /* Set reload-timer period */
    TMR4_PWM_SetFilterCountValue(TMR4_UNIT, TMR4_PWM_CH, TMR4_PWM_RT_VALUE(stcTmr4PwmInit.u16ClockDiv));

    /* Set reload-timer period */
    TMR4_IntCmd(TMR4_UNIT, TMR4_INT_RELOAD_TMR, ENABLE);

    /* Register IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = TMR4_PWM_RT_IRQn;
    stcIrqConfig.enIntSrc = TMR4_PWM_RT_INT_SRC;
    stcIrqConfig.pfnCallback = &TMR4_PWMReloadTimer_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqConfig);
    NVIC_ClearPendingIRQ(stcIrqConfig.enIRQn);
    NVIC_SetPriority(stcIrqConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqConfig.enIRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Start pwm count */
    TMR4_PWM_StartReloadTimer(TMR4_UNIT, TMR4_PWM_CH);

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
