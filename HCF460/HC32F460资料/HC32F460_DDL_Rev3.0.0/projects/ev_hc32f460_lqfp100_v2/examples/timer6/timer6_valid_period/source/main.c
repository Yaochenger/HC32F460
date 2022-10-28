/**
 *******************************************************************************
 * @file  timer6/timer6_valid_period/source/main.c
 * @brief This example demonstrates Timer6 compare output valid period function.
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
 * @addtogroup TIMER6_Valid_Period
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

#define SCMA_ValidPeriod                (1U)

#define TMR6_1_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_08)

#define TEST_GPIO_PORT                  (GPIO_PORT_B)
#define TEST_GPIO_PIN                   (GPIO_PIN_08)

/*
 * ADC unit instance for this example.
 * Macro 'ADC_UNIT' can be defined as CM_ADCx(x=1, 2, 3).
 * Definition of 'ADC_PERIPH_CLK' depends on 'ADC_UNIT'.
 */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1)

/* Specify the ADC channels according to the application.
   Selects any channel you need. */
#define ADC_CH                          (ADC_CH3)
#define ADC_CH_PORT                     (GPIO_PORT_A)
#define ADC_CH_PIN                      (GPIO_PIN_03)

/* Hard trigger */
#define ADC_SEQA_HARDTRIG               (ADC_HARDTRIG_EVT0)
#define ADC_SEQA_AOS_TRIG_SEL           (AOS_ADC1_0)
#define ADC_SEQA_TRIG_EVT               (EVT_SRC_TMR6_1_SCMP_A)

/* Interrupt */
#define ADC_INT_SRC                     (INT_SRC_ADC1_EOCA)
#define ADC_INT_IRQn                    (INT016_IRQn)
#define ADC_INT_PRIO                    (DDL_IRQ_PRIO_03)

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
 * @brief  ADC interrupt handler callback.
 * @param  None
 * @retval None
 */
void ADC1_SeqA_IrqHandler()
{
    GPIO_TogglePins(TEST_GPIO_PORT, TEST_GPIO_PIN);
}

/**
 * @brief  Initializes ADC.
 * @param  None
 * @retval None
 */
static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;
    stc_gpio_init_t stcGpioInit;

    /* 1. Enable ADC peripheral clock. */
    FCG_Fcg3PeriphClockCmd(ADC_PERIPH_CLK, ENABLE);

    /* 2. Modify the default value depends on the application. Not needed here. */
    /* Set a default value. */
    (void)ADC_StructInit(&stcAdcInit);

    /* 3. Initializes ADC. */
    (void)ADC_Init(ADC_UNIT, &stcAdcInit);

    /* 4. ADC channel configuration. */
    /* 4.1 Set the ADC pin to analog input mode. */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(ADC_CH_PORT, ADC_CH_PIN, &stcGpioInit);
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ_A, ADC_CH, ENABLE);
}

/**
 * @brief  ADC IRQ configuration.
 * @param  None
 * @retval None
 */
static void AdcIrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = ADC_INT_SRC;
    stcIrq.enIRQn      = ADC_INT_IRQn;
    stcIrq.pfnCallback = &ADC1_SeqA_IrqHandler;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, ADC_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    ADC_IntCmd(ADC_UNIT, ADC_INT_EOCA, ENABLE);
}

/**
 * @brief  ADC hard trigger configuration.
 * @param  None
 * @retval None
 */
static void AdcHardTriggerConfig(void)
{
    /* Specifies the event defined by 'ADC_SEQA_TRIG_EVT' as the hard trigger of sequence A. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    ADC_TriggerConfig(ADC_UNIT, ADC_SEQ_A, ADC_SEQA_HARDTRIG);
    AOS_SetTriggerEventSrc(ADC_SEQA_AOS_TRIG_SEL, ADC_SEQA_TRIG_EVT);
    ADC_TriggerCmd(ADC_UNIT, ADC_SEQ_A, ENABLE);
}

/**
 * @brief  Configures ADC clock.
 * @param  None
 * @retval None
 */
static void AdcClockConfig(void)
{
    /*
     * 1. Configures the clock divider of PCLK2 and PCLK4 here or in the function of configuring the system clock.
     *    In this example, the system clock is MRC@8MHz.
     *    PCLK4 is the digital interface clock, and PCLK2 is the analog circuit clock.
     *    Make sure that PCLK2 and PCLK4 meet the following conditions:
     *      PCLK4 : PCLK2 = 1:1, 2:1, 4:1, 8:1, 1:2, 1:4.
     *      PCLK2 is in range [1MHz, 60MHz].
     */
    CLK_SetClockDiv((CLK_BUS_PCLK2 | CLK_BUS_PCLK4), (CLK_PCLK2_DIV8 | CLK_PCLK4_DIV2));
    /* 2. Specifies the clock source of ADC. */
    CLK_SetPeriClockSrc(CLK_PERIPHCLK_PCLK);
}

/**
 * @brief  ADC configuration for example
 * @param  None
 * @retval None
 */
void AdcConfig(void)
{
    AdcClockConfig();
    AdcInitConfig();
    AdcIrqConfig();
    AdcHardTriggerConfig();
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
    stc_tmr6_valid_period_config_t stcValidPeriodConfig;
    stc_gpio_init_t stcGpioCfg;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();

    (void)TMR6_StructInit(&stcTmr6Init);
    (void)TMR6_PWM_StructInit(&stcPwmInit);
    (void)TMR6_ValidPeriodStructInit(&stcValidPeriodConfig);

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1, ENABLE);

    /* Timer6 PWM output port configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);

    /* Configate GPIO for example */
    (void)GPIO_StructInit(&stcGpioCfg);
    stcGpioCfg.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(TEST_GPIO_PORT, TEST_GPIO_PIN, &stcGpioCfg);

    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32CountMode = TMR6_MD_TRIANGLE_A;
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV1024;
    stcTmr6Init.u32PeriodValue = HCLK_VALUE / 1024U / 4U;  /* Count for 250ms */
    (void)TMR6_Init(CM_TMR6_1, &stcTmr6Init);

    /* Set special compare register */
    TMR6_SetSpecialCompareValue(CM_TMR6_1, TMR6_CMP_REG_A, 0x3000U);

    /* Configurate PWM output */
    stcPwmInit.u32CompareValue = 0x3000U;
    stcPwmInit.u32PeriodMatchPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CompareMatchPolarity = TMR6_PWM_INVT;
    stcPwmInit.u32StopPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartStopHold = TMR6_PWM_START_STOP_CHANGE;
    (void)TMR6_PWM_Init(CM_TMR6_1, TMR6_CH_A, &stcPwmInit);
    /* PWM pin function set */
    TMR6_SetFunc(CM_TMR6_1, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    /* PWM output command */
    TMR6_PWM_OutputCmd(CM_TMR6_1, TMR6_CH_A, ENABLE);

#if (SCMA_ValidPeriod == 1U)
    /* Valid period function configurate */
    stcValidPeriodConfig.u32CountCond = TMR6_VALID_PERIOD_CNT_COND_OVF_UDF;
    stcValidPeriodConfig.u32PeriodInterval = TMR6_VALID_PERIOD_CNT5;
    (void)TMR6_ValidPeriodConfig(CM_TMR6_1, &stcValidPeriodConfig);
    TMR6_ValidPeriodCmd(CM_TMR6_1, TMR6_CH_A, ENABLE);
#endif

    /* Enable interrupt */
    TMR6_IntCmd(CM_TMR6_1, TMR6_INT_UP_CNT_SPECIAL_MATCH_A | TMR6_INT_DOWN_CNT_SPECIAL_MATCH_A, ENABLE);

    AdcConfig();

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
