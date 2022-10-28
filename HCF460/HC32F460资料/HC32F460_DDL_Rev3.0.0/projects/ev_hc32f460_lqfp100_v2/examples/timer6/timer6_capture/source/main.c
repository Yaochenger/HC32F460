/**
 *******************************************************************************
 * @file  timer6/timer6_capture/source/main.c
 * @brief This example demonstrates Timer6 Capture function.
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
 * @addtogroup TIMER6_Capture
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

#define EXAMPLE_PWM_IRQN                (INT002_IRQn)
#define EXAMPLE_PWM_INT_SRC             (INT_SRC_TMR6_1_OVF)

#define EXAMPLE_CAPT_IRQN               (INT003_IRQn)
#define EXAMPLE_CAPT_INT_SRC            (INT_SRC_TMR6_2_GCMP_A)

#define TMR6_1_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_08)

#define TMR6_2_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_2_PWMA_PIN                 (GPIO_PIN_09)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint32_t u32CaptureA;

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
    static uint8_t i = 0U;

    if (0U == i) {
        TMR6_SetCompareValue(CM_TMR6_1, TMR6_CMP_REG_E, 0x4000U);
        i = 1U;
    } else if (1U == i) {
        TMR6_SetCompareValue(CM_TMR6_1, TMR6_CMP_REG_E, 0x6000U);
        i = 2U;
    } else {
        TMR6_SetCompareValue(CM_TMR6_1, TMR6_CMP_REG_E, 0x2000U);
        i = 0U;
    }
}

/**
 * @brief  TIMER6 capture input interrupt handler callback.
 * @param  None
 * @retval None
 */
static void Tmr6_2_CapInputCallBack(void)
{
    u32CaptureA = TMR6_GetCompareValue(CM_TMR6_2, TMR6_CMP_REG_A);
    DDL_Printf("0x%x\r\n", (unsigned int)u32CaptureA);
}

/**
 * @brief  Config CM_TMR6_1 PWM output
 * @param  [in] TMR6x               Timer6 unit
 *  @arg CM_TMR6_x
 * @retval None
 */
static void ConfigPWM(CM_TMR6_TypeDef *TMR6x)
{
    stc_timer6_init_t stcTmr6Init;
    stc_tmr6_pwm_init_t stcPwmInit;
    stc_irq_signin_config_t stcIrqRegiConf;

    (void)TMR6_StructInit(&stcTmr6Init);
    (void)TMR6_PWM_StructInit(&stcPwmInit);

    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV1024;
    stcTmr6Init.u32PeriodValue = 0xFFFFU;
    (void)TMR6_Init(TMR6x, &stcTmr6Init);

    /* General compare buffer function configurate */
    TMR6_SetGeneralBufNum(TMR6x, TMR6_CH_A, TMR6_BUF_DUAL);
    TMR6_GeneralBufCmd(TMR6x, TMR6_CH_A, ENABLE);
    /* Compare register set */
    TMR6_SetCompareValue(TMR6x, TMR6_CMP_REG_C, 0x4000U); /* Set General Compare RegisterC Value */
    TMR6_SetCompareValue(TMR6x, TMR6_CMP_REG_E, 0x6000U); /* Set General Compare RegisterE Value */

    /* Configurate PWM output */
    stcPwmInit.u32CompareValue = 0x2000U;
    stcPwmInit.u32PeriodMatchPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32CompareMatchPolarity = TMR6_PWM_HIGH;
    stcPwmInit.u32StopPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartStopHold = TMR6_PWM_START_STOP_CHANGE;
    (void)TMR6_PWM_Init(TMR6x, TMR6_CH_A, &stcPwmInit);
    /* PWM pin function set */
    TMR6_SetFunc(TMR6x, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    /* PWM output command */
    TMR6_PWM_OutputCmd(TMR6x, TMR6_CH_A, ENABLE);

    /* config interrupt */
    TMR6_IntCmd(TMR6x, TMR6_INT_OVF, ENABLE);
    stcIrqRegiConf.enIRQn = EXAMPLE_PWM_IRQN;
    stcIrqRegiConf.enIntSrc = EXAMPLE_PWM_INT_SRC;
    stcIrqRegiConf.pfnCallback = &Tmr6_OverFlow_CallBack;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}

/**
 * @brief  Config CM_TMR6_2 PWM capture input
 * @param  [in] TMR6x               Timer6 unit
 *  @arg CM_TMR6_x
 * @retval None
 */
static void ConfigCapture(CM_TMR6_TypeDef *TMR6x)
{
    stc_timer6_init_t stcTmr6Init;
    stc_irq_signin_config_t stcIrqRegiConf;

    (void)TMR6_StructInit(&stcTmr6Init);

    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV1024;
    stcTmr6Init.u32PeriodValue = 0xFFFFu;
    (void)TMR6_Init(TMR6x, &stcTmr6Init);

    /* Capture input pin configuration */
    (void)TMR6_SetFilterClockDiv(TMR6x, TMR6_IO_PWMA, TMR6_FILTER_CLK_DIV16);
    TMR6_FilterCmd(TMR6x, TMR6_IO_PWMA, ENABLE);
    TMR6_SetFunc(TMR6x, TMR6_CH_A, TMR6_PIN_CAPT_INPUT);

    /* Hardware capture: Timer6 PWMA pin rising */
    TMR6_HWCaptureCondCmd(TMR6x, TMR6_CH_A, TMR6_CAPT_COND_PWMA_RISING, ENABLE);
    /* HW Clear: Timer6 PWMA pin fall edge */
    TMR6_HWClearCondCmd(TMR6x, TMR6_CLR_COND_PWMA_FAILLING, ENABLE);
    TMR6_HWClearCmd(TMR6x, ENABLE);

    /* Enable CM_TMR6_2 GCMAR interrupt */
    TMR6_IntCmd(TMR6x, TMR6_INT_MATCH_A, ENABLE);
    stcIrqRegiConf.enIRQn = EXAMPLE_CAPT_IRQN;
    stcIrqRegiConf.enIntSrc = EXAMPLE_CAPT_INT_SRC;
    stcIrqRegiConf.pfnCallback = &Tmr6_2_CapInputCallBack;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIO_04);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}

/**
 * @brief  Main function of TIMER6 compare output mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    DDL_Printf("The capture value: \r\n");

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1, ENABLE);
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_2, ENABLE);

    /* Timer6 PWM output pin configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_2_PWMA_PORT, TMR6_2_PWMA_PIN, GPIO_FUNC_3);

    /* Config CM_TMR6_1 PWM output */
    ConfigPWM(CM_TMR6_1);

    /* Config CM_TMR6_2 capture in */
    ConfigCapture(CM_TMR6_2);

    /* Start timer6 */
    TMR6_SWSyncStart(TMR6_SW_SYNC_U1 | TMR6_SW_SYNC_U2);

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
