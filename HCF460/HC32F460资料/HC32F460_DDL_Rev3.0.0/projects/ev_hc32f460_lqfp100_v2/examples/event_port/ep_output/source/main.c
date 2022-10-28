/**
 *******************************************************************************
 * @file  event_port/ep_output/source/main.c
 * @brief Main program Event Port Ouput for the Device Driver Library.
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
 * @addtogroup EVENT_PORT_OUTPUT
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define LED_G_PORT          (EVT_PORT_4)
#define LED_G_PIN           (EVT_PIN_04)
#define EP_TRIG_SEL         (AOS_EVTPORT34)
#define EP_TRIG_SRC         (EVT_SRC_TMR0_1_CMP_A)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void Timer0_Init(void);
static void EventPort_Init(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Event port initialize
 * @param  None
 * @retval None
 */
static void EventPort_Init(void)
{
    stc_ep_init_t stcEPInit;
    (void)EP_StructInit(&stcEPInit);

    FCG_Fcg0PeriphClockCmd(PWC_FCG0_AOS, ENABLE);

    GPIO_SetFunc(LED_G_PORT, LED_G_PIN, GPIO_FUNC_14);

    stcEPInit.enPinState        = EVT_PIN_RESET;
    stcEPInit.u32PinDir         = EP_DIR_OUT;
    stcEPInit.u32PinTriggerOps  = EP_OPS_TOGGLE;
    (void)EP_Init(LED_G_PORT, LED_G_PIN, &stcEPInit);

    AOS_SetTriggerEventSrc(EP_TRIG_SEL, EP_TRIG_SRC);
}

/**
 * @brief  Timer 0 initialize
 * @param  None
 * @retval None
 */
static void Timer0_Init(void)
{
    stc_tmr0_init_t stcTmr0Init;

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR0_1, ENABLE);
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV1024;
    (void)TMR0_Init(CM_TMR0_1, TMR0_CH_A, &stcTmr0Init);
}

/**
 * @brief  Main function of EventPort output project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
    /* BSP Clock initialize */
    BSP_CLK_Init();
    /* Event Port init */
    EventPort_Init();
    /* Timer 0 init */
    Timer0_Init();
    /* Start timer 0 */
    TMR0_Start(CM_TMR0_1, TMR0_CH_A);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
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
