/**
 *******************************************************************************
 * @file  timer6/timer6_cmp_sawtooth/source/main.c
 * @brief This example demonstrates Timer6 compare output.
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
 * @addtogroup TIMER6_Cmp_Sawtooth
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

#define TMR6_1_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_08)

#define TMR6_1_PWMB_PORT                (GPIO_PORT_E)
#define TMR6_1_PWMB_PIN                 (GPIO_PIN_08)
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
 * @brief  TIMER6 overflow interrupt handler callback.
 * @param  None
 * @retval None
 */
static void Tmr6_OverFlow_CallBack(void)
{
    BSP_LED_Toggle(LED_BLUE);
}

/**
 * @brief  Main function of TIMER6 compare output mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_timer6_init_t stcTmr6Init;
    stc_tmr6_pwm_init_t stcPwmInit;
    stc_gpio_init_t stcGpioInit;
    stc_irq_signin_config_t stcIrqRegiConf;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();

    (void)TMR6_StructInit(&stcTmr6Init);
    (void)TMR6_PWM_StructInit(&stcPwmInit);
    (void)GPIO_StructInit(&stcGpioInit);

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1, ENABLE);

    /* Timer6 PWM output port configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_1_PWMB_PORT, TMR6_1_PWMB_PIN, GPIO_FUNC_3);

    TMR6_Stop(CM_TMR6_1);
    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV1024;
    stcTmr6Init.u32PeriodValue = HCLK_VALUE / 1024U / 4U;  /* Count for 250ms */
    (void)TMR6_Init(CM_TMR6_1, &stcTmr6Init);

    /* Configurate PWM output */
    stcPwmInit.u32CompareValue = 0x1000UL;
    stcPwmInit.u32PeriodMatchPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32CompareMatchPolarity = TMR6_PWM_HIGH;
    stcPwmInit.u32StopPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartStopHold = TMR6_PWM_START_STOP_CHANGE;
    (void)TMR6_PWM_Init(CM_TMR6_1, TMR6_CH_A, &stcPwmInit);
    stcPwmInit.u32CompareValue = 0x2000UL;
    stcPwmInit.u32PeriodMatchPolarity = TMR6_PWM_HIGH;
    stcPwmInit.u32CompareMatchPolarity = TMR6_PWM_LOW;
    (void)TMR6_PWM_Init(CM_TMR6_1, TMR6_CH_B, &stcPwmInit);
    /* PWM pin function set */
    TMR6_SetFunc(CM_TMR6_1, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_1, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    /* PWM output command */
    TMR6_PWM_OutputCmd(CM_TMR6_1, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_1, TMR6_CH_B, ENABLE);

    stcIrqRegiConf.enIRQn = INT002_IRQn;
    stcIrqRegiConf.enIntSrc = INT_SRC_TMR6_1_OVF;
    stcIrqRegiConf.pfnCallback = &Tmr6_OverFlow_CallBack;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    /* Enable interrupt */
    TMR6_IntCmd(CM_TMR6_1, TMR6_INT_OVF, ENABLE);
    /* Start timer6 */
    TMR6_Start(CM_TMR6_1);

    for (;;) {
        ;
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
