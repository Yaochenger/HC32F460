/**
 *******************************************************************************
 * @file  timer0/timer0_capture/source/main.c
 * @brief Main program of TIMER0 for the Device Driver Library.
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
 * @addtogroup TIMER0_Capture
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

/* TMR0 unit and channel definition */
#define TMR0_UNIT                       (CM_TMR0_2)
#define TMR0_CLK                        (FCG2_PERIPH_TMR0_2)
#define TMR0_CH                         (TMR0_CH_A)
#define TMR0_TRIG_CH                    (AOS_TMR0)
#define TMR0_CH_INT                     (TMR0_INT_CMP_A)
#define TMR0_CH_FLAG                    (TMR0_FLAG_CMP_A)
#define TMR0_INT_SRC                    (INT_SRC_TMR0_2_CMP_A)
#define TMR0_IRQn                       (INT006_IRQn)

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
 * @brief  TMR0 capture interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMR0_CaptureIrqCallback(void)
{
    BSP_LED_Toggle(LED_BLUE);
    TMR0_ClearStatus(TMR0_UNIT, TMR0_CH_FLAG);
}

/**
 * @brief  Configure TMR0.
 * @note   In asynchronous clock, If you want to write a TMR0 register, you need to wait for
 *         at least 3 asynchronous clock cycles after the last write operation!
 * @param  None
 * @retval None
 */
static void TMR0_Config(void)
{
    stc_tmr0_init_t stcTmr0Init;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Enable timer0 and AOS clock */
    FCG_Fcg2PeriphClockCmd(TMR0_CLK, ENABLE);
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

    /* TIMER0 initialize */
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC_INTERN_CLK;
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV1024;
    stcTmr0Init.u32Func     = TMR0_FUNC_CAPT;
    (void)TMR0_Init(TMR0_UNIT, TMR0_CH, &stcTmr0Init);
    TMR0_IntCmd(TMR0_UNIT, TMR0_CH_INT, ENABLE);
    AOS_SetTriggerEventSrc(TMR0_TRIG_CH, BSP_KEY_KEY10_EVT);

    /* Interrupt configuration */
    stcIrqSignConfig.enIntSrc    = TMR0_INT_SRC;
    stcIrqSignConfig.enIRQn      = TMR0_IRQn;
    stcIrqSignConfig.pfnCallback = &TMR0_CaptureIrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
}

/**
 * @brief  Main function of example project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* BSP init */
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Configure TMR0 */
    TMR0_Config();
    /* TMR0 start counting */
    TMR0_Start(TMR0_UNIT, TMR0_CH);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

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
