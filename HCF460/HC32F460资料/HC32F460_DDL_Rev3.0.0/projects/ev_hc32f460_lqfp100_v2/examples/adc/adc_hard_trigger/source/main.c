/**
 *******************************************************************************
 * @file  adc/adc_hard_trigger/source/main.c
 * @brief Main program ADC hard trigger for the Device Driver Library.
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
 * @addtogroup ADC_Hard_Trigger
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* ADC unit instance for this example. */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1)

/* Selects ADC channels that needed. */
#define ADC_SEQA_CH                     (ADC_CH10)
#define ADC_SEQA_CH_PORT                (GPIO_PORT_C)
#define ADC_SEQA_CH_PIN                 (GPIO_PIN_00)

#define ADC_SEQB_CH                     (ADC_CH12)
#define ADC_SEQB_CH_PORT                (GPIO_PORT_C)
#define ADC_SEQB_CH_PIN                 (GPIO_PIN_02)

/* Hard trigger */
#define ADC_SEQA_HARDTRIG               (ADC_HARDTRIG_ADTRG_PIN)
#define ADC_SEQA_TRIG_PORT              (GPIO_PORT_E)
#define ADC_SEQA_TRIG_PIN               (GPIO_PIN_07)
#define ADC_SEQA_TRIG_PIN_FUNC          (GPIO_FUNC_1)

#define ADC_SEQB_HARDTRIG               (ADC_HARDTRIG_EVT0)
#define ADC_SEQB_AOS_TRIG_SEL           (AOS_ADC1_0)
#define ADC_SEQB_TRIG_EVT               (EVT_SRC_TMR0_1_CMP_B)

/* Timer0 for sequence B */
#define TMR0_UNIT                       (CM_TMR0_1)
#define TMR0_CH                         (TMR0_CH_B)
#define TMR0_PERIPH_CLK                 (FCG2_PERIPH_TMR0_1)
#define TMR0_CMP_VAL                    (31250UL - 1UL)
#define TMR0_CLK_DIV                    (TMR0_CLK_DIV256)
#define TMR0_PERIPH_ENABLE()            FCG_Fcg2PeriphClockCmd(TMR0_PERIPH_CLK, ENABLE)

/* Interrupt */
#define ADC_SEQA_INT_PRIO               (DDL_IRQ_PRIO_04)
#define ADC_SEQA_INT_SRC                (INT_SRC_ADC1_EOCA)
#define ADC_SEQA_INT_IRQn               (INT116_IRQn)

#define ADC_SEQB_INT_PRIO               (DDL_IRQ_PRIO_03)
#define ADC_SEQB_INT_SRC                (INT_SRC_ADC1_EOCB)
#define ADC_SEQB_INT_IRQn               (INT117_IRQn)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void AdcConfig(void);
static void AdcInitConfig(void);
static void AdcSetPinAnalogMode(void);
static void AdcHardTriggerConfig(void);
static void AdcIrqConfig(void);

static void AdcHardTriggerStart(void);

static void ADC1_SeqA_IrqCallback(void);
static void ADC1_SeqB_IrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of ADC hard trigger project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* System clock is MRC@8MHz */

    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Initializes UART for debug printing. Baudrate is 19200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 19200U, BSP_PRINTF_Preinit);
    /* Configures ADC. */
    AdcConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    /* Start the peripheral which is used to trigger ADC. */
    AdcHardTriggerStart();

    /***************** Configuration end, application start **************/

    for (;;) {
        /* See ADC1_SeqA_IrqCallback(), ADC1_SeqB_IrqCallback() */
    }
}

/**
 * @brief  ADC configuration.
 * @param  None
 * @retval None
 */
static void AdcConfig(void)
{
    AdcInitConfig();
    AdcHardTriggerConfig();
    AdcIrqConfig();
}

/**
 * @brief  Initializes ADC.
 * @param  None
 * @retval None
 */
static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;

    /* 1. Enable ADC peripheral clock. */
    FCG_Fcg3PeriphClockCmd(ADC_PERIPH_CLK, ENABLE);

    /* 2. Modify the default value depends on the application. */
    (void)ADC_StructInit(&stcAdcInit);
    stcAdcInit.u16ScanMode = ADC_MD_SEQA_SEQB_SINGLESHOT;

    /* 3. Initializes ADC. */
    (void)ADC_Init(ADC_UNIT, &stcAdcInit);

    /* 4. ADC channel configuration. */
    /* 4.1 Set the ADC pin to analog input mode. */
    AdcSetPinAnalogMode();
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ_A, ADC_SEQA_CH, ENABLE);
    ADC_ChCmd(ADC_UNIT, ADC_SEQ_B, ADC_SEQB_CH, ENABLE);
}

/**
 * @brief  Set specified ADC pin to analog mode.
 * @param  None
 * @retval None
 */
static void AdcSetPinAnalogMode(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(ADC_SEQA_CH_PORT, ADC_SEQA_CH_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC_SEQB_CH_PORT, ADC_SEQB_CH_PIN, &stcGpioInit);
}

/**
 * @brief  ADC hard trigger configuration.
 * @param  None
 * @retval None
 */
static void AdcHardTriggerConfig(void)
{
    stc_tmr0_init_t stcTmr0Init;

    /************** Hard trigger of sequence A ****************/
    GPIO_SetFunc(ADC_SEQA_TRIG_PORT, ADC_SEQA_TRIG_PIN, ADC_SEQA_TRIG_PIN_FUNC);
    ADC_TriggerConfig(ADC_UNIT, ADC_SEQ_A, ADC_SEQA_HARDTRIG);
    ADC_TriggerCmd(ADC_UNIT, ADC_SEQ_A, ENABLE);

    /************** Hard trigger of sequence B ****************/
    /* Initials TIMER0. */
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockDiv     = TMR0_CLK_DIV;
    stcTmr0Init.u16CompareValue = (uint16_t)TMR0_CMP_VAL;

    TMR0_PERIPH_ENABLE();
    (void)TMR0_Init(TMR0_UNIT, TMR0_CH, &stcTmr0Init);

    /* Specifies the event defined by 'ADC_SEQB_TRIG_EVT' as the hard trigger of sequence B. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    ADC_TriggerConfig(ADC_UNIT, ADC_SEQ_B, ADC_SEQB_HARDTRIG);
    AOS_SetTriggerEventSrc(ADC_SEQB_AOS_TRIG_SEL, ADC_SEQB_TRIG_EVT);
    ADC_TriggerCmd(ADC_UNIT, ADC_SEQ_B, ENABLE);
}

/**
 * @brief  Start the peripheral which is used to trigger ADC.
 * @param  None
 * @retval None
 */
static void AdcHardTriggerStart(void)
{
    /************** Hard trigger of sequence A ****************/
    /* Not needed here. */

    /************** Hard trigger of sequence B ****************/
    TMR0_Start(TMR0_UNIT, TMR0_CH);
}

/**
 * @brief  ADC IRQ configuration.
 * @param  None
 * @retval None
 */
static void AdcIrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = ADC_SEQA_INT_SRC;
    stcIrq.enIRQn      = ADC_SEQA_INT_IRQn;
    stcIrq.pfnCallback = &ADC1_SeqA_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, ADC_SEQA_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    stcIrq.enIntSrc    = ADC_SEQB_INT_SRC;
    stcIrq.enIRQn      = ADC_SEQB_INT_IRQn;
    stcIrq.pfnCallback = &ADC1_SeqB_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, ADC_SEQB_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    ADC_IntCmd(ADC_UNIT, ADC_INT_EOCA | ADC_INT_EOCB, ENABLE);
}

/**
 * @brief  ADC sequence A interrupt callback.
 * @param  None
 * @retval None
 */
static void ADC1_SeqA_IrqCallback(void)
{
    uint16_t u16AdcVal;
    u16AdcVal = ADC_GetValue(ADC_UNIT, ADC_SEQA_CH);
    ADC_ClearStatus(ADC_UNIT, ADC_FLAG_EOCA);

    DDL_Printf("ADC sequence A was triggered by ADTRG pin.\r\n");
    DDL_Printf("ADC value of sequence A channel: %u\r\n", u16AdcVal);
}

/**
 * @brief  ADC sequence B interrupt callback.
 * @param  None
 * @retval None
 */
static void ADC1_SeqB_IrqCallback(void)
{
    uint16_t u16AdcVal;
    u16AdcVal = ADC_GetValue(ADC_UNIT, ADC_SEQB_CH);
    ADC_ClearStatus(ADC_UNIT, ADC_FLAG_EOCB);

    DDL_Printf("ADC sequence B was triggered by TMR0 event.\r\n");
    DDL_Printf("ADC value of sequence B channel: %u\r\n", u16AdcVal);
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
