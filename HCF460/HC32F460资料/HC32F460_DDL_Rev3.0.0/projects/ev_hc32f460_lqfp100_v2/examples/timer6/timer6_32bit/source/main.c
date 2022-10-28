/**
 *******************************************************************************
 * @file  timer6/timer6_32bit/source/main.c
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
 * @addtogroup TIMER6_32bit
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
 * @brief  Main function of project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_timer6_init_t stcTmr6Init;
    stc_gpio_init_t stcGpioInit;
    stc_irq_signin_config_t stcIrqRegiConf;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();

    (void)TMR6_StructInit(&stcTmr6Init);
    (void)GPIO_StructInit(&stcGpioInit);

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1, ENABLE);
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_2, ENABLE);
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

    TMR6_Stop(CM_TMR6_1);
    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV64;
    stcTmr6Init.u32PeriodValue = HCLK_VALUE / 64U / 1000U;  /* Count for 1ms */
    (void)TMR6_Init(CM_TMR6_1, &stcTmr6Init);

    /* Timer6 trigger event set */
    AOS_SetTriggerEventSrc(AOS_TMR6_0, EVT_SRC_TMR6_1_OVF);

    TMR6_Stop(CM_TMR6_2);
    /* Config hardware count */
    (void)TMR6_StructInit(&stcTmr6Init);
    stcTmr6Init.u8CountSrc = TMR6_CNT_SRC_HW;
    stcTmr6Init.hw_count.u32CountUpCond = TMR6_CNT_UP_COND_EVT0;
    stcTmr6Init.u32PeriodValue = 500U;  /* Count for 500ms */
    (void)TMR6_Init(CM_TMR6_2, &stcTmr6Init);

    /* Enable overflow interrupt */
    TMR6_IntCmd(CM_TMR6_2, TMR6_INT_OVF, ENABLE);

    stcIrqRegiConf.enIRQn = INT003_IRQn;
    stcIrqRegiConf.enIntSrc = INT_SRC_TMR6_2_OVF;
    stcIrqRegiConf.pfnCallback = &Tmr6_OverFlow_CallBack;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    /*start timer6*/
    TMR6_Start(CM_TMR6_2);
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
