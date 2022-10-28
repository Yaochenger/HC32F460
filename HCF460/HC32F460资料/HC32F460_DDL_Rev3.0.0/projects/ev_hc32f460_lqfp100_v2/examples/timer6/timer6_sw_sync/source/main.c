/**
 *******************************************************************************
 * @file  timer6/timer6_sw_sync/source/main.c
 * @brief This example demonstrates Timer6 software synchronize trigger function.
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
 * @addtogroup TIMER6_Software_Synchronous
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU |\
                                         LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

#define EXAMPLE_IRQN                    (INT002_IRQn)
#define EXAMPLE_TMR6_INT_SRC            (INT_SRC_TMR6_1_UDF)

#define TMR6_1_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_08)
#define TMR6_1_PWMB_PORT                (GPIO_PORT_E)
#define TMR6_1_PWMB_PIN                 (GPIO_PIN_08)

#define TMR6_2_PWMA_PORT                (GPIO_PORT_E)
#define TMR6_2_PWMA_PIN                 (GPIO_PIN_11)
#define TMR6_2_PWMB_PORT                (GPIO_PORT_E)
#define TMR6_2_PWMB_PIN                 (GPIO_PIN_10)

#define TMR6_3_PWMA_PORT                (GPIO_PORT_E)
#define TMR6_3_PWMA_PIN                 (GPIO_PIN_13)
#define TMR6_3_PWMB_PORT                (GPIO_PORT_B)
#define TMR6_3_PWMB_PIN                 (GPIO_PIN_15)

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
 * @brief  TIMER6 interrupt handler callback.
 * @param  None
 * @retval None
 */
static void Tmr6_UnderFlow_CallBack(void)
{
    static uint8_t i = 0U;

    if (0U == i) {
        TMR6_SetCompareValue(CM_TMR6_1, TMR6_CMP_REG_C, 0x2000U);
        TMR6_SetCompareValue(CM_TMR6_2, TMR6_CMP_REG_C, 0x2000U);
        TMR6_SetCompareValue(CM_TMR6_3, TMR6_CMP_REG_C, 0x2000U);
        i = 1U;
    } else {
        TMR6_SetCompareValue(CM_TMR6_1, TMR6_CMP_REG_C, 0x4000U);
        TMR6_SetCompareValue(CM_TMR6_2, TMR6_CMP_REG_C, 0x4000U);
        TMR6_SetCompareValue(CM_TMR6_3, TMR6_CMP_REG_C, 0x4000U);

        i = 0U;
    }
}

/**
 * @brief  Main function of TIMER6 compare output mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t u32Compare;
    stc_timer6_init_t stcTmr6Init;
    stc_tmr6_pwm_init_t stcPwmInit;
    stc_irq_signin_config_t stcIrqConfig;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();

    (void)TMR6_StructInit(&stcTmr6Init);
    (void)TMR6_PWM_StructInit(&stcPwmInit);

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1, ENABLE);
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_2, ENABLE);
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_3, ENABLE);

    /* Timer6 PWM pin configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_1_PWMB_PORT, TMR6_1_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_2_PWMA_PORT, TMR6_2_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_2_PWMB_PORT, TMR6_2_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_3_PWMA_PORT, TMR6_3_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_3_PWMB_PORT, TMR6_3_PWMB_PIN, GPIO_FUNC_3);

    TMR6_DeInit(CM_TMR6_1);
    TMR6_DeInit(CM_TMR6_2);
    TMR6_DeInit(CM_TMR6_3);

    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32CountMode = TMR6_MD_TRIANGLE_A;
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV1;
    stcTmr6Init.u32PeriodValue = 0x8340U;
    (void)TMR6_Init(CM_TMR6_1, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_2, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_3, &stcTmr6Init);

    /* General compare buffer function configurate */
    TMR6_SetGeneralBufNum(CM_TMR6_1, TMR6_CH_A, TMR6_BUF_DUAL);
    TMR6_SetGeneralBufNum(CM_TMR6_2, TMR6_CH_A, TMR6_BUF_DUAL);
    TMR6_SetGeneralBufNum(CM_TMR6_3, TMR6_CH_A, TMR6_BUF_DUAL);
    /* General compare buffer function command */
    TMR6_GeneralBufCmd(CM_TMR6_1, TMR6_CH_A, ENABLE);
    TMR6_GeneralBufCmd(CM_TMR6_2, TMR6_CH_A, ENABLE);
    TMR6_GeneralBufCmd(CM_TMR6_3, TMR6_CH_A, ENABLE);
    /* Compare register set */
    u32Compare = 0x2000U;
    TMR6_SetCompareValue(CM_TMR6_1, TMR6_CMP_REG_C, u32Compare);
    TMR6_SetCompareValue(CM_TMR6_2, TMR6_CMP_REG_C, u32Compare);
    TMR6_SetCompareValue(CM_TMR6_3, TMR6_CMP_REG_C, u32Compare);

    /* Configurate PWM output */
    stcPwmInit.u32CompareValue = 0x2000U;
    stcPwmInit.u32PeriodMatchPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32CompareMatchPolarity = TMR6_PWM_HIGH;
    stcPwmInit.u32StopPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartStopHold = TMR6_PWM_START_STOP_CHANGE;
    (void)TMR6_PWM_Init(CM_TMR6_1, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_2, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_3, TMR6_CH_A, &stcPwmInit);
    stcPwmInit.u32CompareValue = 0x6000U;
    stcPwmInit.u32PeriodMatchPolarity = TMR6_PWM_HIGH;
    stcPwmInit.u32CompareMatchPolarity = TMR6_PWM_LOW;
    (void)TMR6_PWM_Init(CM_TMR6_1, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_2, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_3, TMR6_CH_B, &stcPwmInit);

    /* PWM output command */
    TMR6_PWM_OutputCmd(CM_TMR6_1, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_2, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_3, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_1, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_2, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_3, TMR6_CH_B, ENABLE);

    /* Set PWMA PWMB output */
    TMR6_SetFunc(CM_TMR6_1, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_2, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_3, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_1, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_2, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_3, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);

    /* ENABLE interrupt */
    TMR6_IntCmd(CM_TMR6_1, TMR6_INT_UDF, ENABLE);
    /* NVIC */
    stcIrqConfig.enIRQn = EXAMPLE_IRQN;
    stcIrqConfig.enIntSrc = EXAMPLE_TMR6_INT_SRC;
    stcIrqConfig.pfnCallback = &Tmr6_UnderFlow_CallBack;
    (void)INTC_IrqSignIn(&stcIrqConfig);
    NVIC_ClearPendingIRQ(EXAMPLE_IRQN);
    NVIC_SetPriority(EXAMPLE_IRQN, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(EXAMPLE_IRQN);

    for (;;) {
        /* Start timer6 */
        TMR6_SWSyncStart(TMR6_SW_SYNC_ALL);
        DDL_DelayMS(500UL);

        /* Stop timer6 */
        TMR6_SWSyncStop(TMR6_SW_SYNC_ALL);
        DDL_DelayMS(500UL);

        /* Clear timer6 count register */
        TMR6_SWSyncClear(TMR6_SW_SYNC_ALL);
        DDL_DelayMS(500UL);
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
