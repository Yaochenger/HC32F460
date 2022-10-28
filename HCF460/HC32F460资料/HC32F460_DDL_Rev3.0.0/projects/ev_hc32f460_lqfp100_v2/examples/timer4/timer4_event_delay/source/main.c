/**
 *******************************************************************************
 * @file  timer4/timer4_event_delay/source/main.c
 * @brief This example demonstrates how to use delay mode of TMR4 event function.
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
 * @addtogroup TIMER4_Event_Delay
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

/* TMR4 count period value(250ms) */
#define TMR4_CNT_PERIOD_VALUE(div)      ((uint16_t)(TMR4_ClockFreq() / (1UL << (uint32_t)(div)) / 4UL) - 1U)

/* TMR4 event channel definition */
#define TMR4_EVT_CH                     (TMR4_EVT_CH_UH)

/* ADC pin definition */
#define ADC_PORT                        (GPIO_PORT_C)
#define ADC_PIN                         (GPIO_PIN_00)

/* ADC unit definitions */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_FCG_ENABLE()                (FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC1, ENABLE))

/* ADC channel definition */
#define ADC_CH                          (ADC_CH10)

/* ADC trigger event source */
#define ADC_TRIG_SEL                    (AOS_ADC1_0)
#define ADC_EVT_SRC                     (EVT_SRC_TMR4_1_SCMP_UH)

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
 * @brief  Get TMR4 clock frequency.
 * @param  None
 * @retval TMR4 clock frequency
 */
static uint32_t TMR4_ClockFreq(void)
{
    return CLK_GetBusClockFreq(CLK_BUS_PCLK1);
}

/**
 * @brief  Initializes ADC.
 * @param  None
 * @retval None
 */
static void ADC_Config(void)
{
    stc_adc_init_t stcAdcInit;
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(ADC_PORT, ADC_PIN, &stcGpioInit);

    /* Enable ADC peripheral clock */
    ADC_FCG_ENABLE();

    /* Initializes ADC. */
    (void)ADC_StructInit(&stcAdcInit);
    (void)ADC_Init(ADC_UNIT, &stcAdcInit);

    /* Enable AOS peripheral clock */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

    /* Specifies the hard trigger for the specified ADC sequence. */
    ADC_TriggerConfig(ADC_UNIT, ADC_SEQ_A, ADC_HARDTRIG_EVT0);

    /* Specifies the trigger event. */
    AOS_SetTriggerEventSrc(ADC_TRIG_SEL, ADC_EVT_SRC);

    /* Enable the sequence A trigger function of ADC. */
    ADC_TriggerCmd(ADC_UNIT, ADC_SEQ_A, ENABLE);

    /* Enable ADC channel. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ_A, ADC_CH, ENABLE);
}

/**
 * @brief  Main function of TMR4 event delay trigger mode
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    static uint16_t m_u16AdcVal = 0U;
    stc_tmr4_init_t stcTmr4Init;
    stc_tmr4_evt_init_t stcTmr4EventInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize UART for debug print function. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);

    /* Enable TMR4 peripheral clock */
    TMR4_FCG_ENABLE();

    /* Configure ADC. */
    ADC_Config();

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /************************* Configure TMR4 counter *************************/
    (void)TMR4_StructInit(&stcTmr4Init);
    stcTmr4Init.u16ClockDiv = TMR4_CLK_DIV1024;
    stcTmr4Init.u16PeriodValue = TMR4_CNT_PERIOD_VALUE(stcTmr4Init.u16ClockDiv);
    (void)TMR4_Init(TMR4_UNIT, &stcTmr4Init);

    /************************* Configure TMR4 event ****************************/
    /* TMR4 event: initialize */
    (void)TMR4_EVT_StructInit(&stcTmr4EventInit);
    stcTmr4EventInit.u16MatchCond = TMR4_EVT_MATCH_CNT_UP;
    stcTmr4EventInit.u16Mode = TMR4_EVT_MD_DELAY;
    stcTmr4EventInit.u16CompareValue = (stcTmr4Init.u16PeriodValue / 4U);
    (void)TMR4_EVT_Init(TMR4_UNIT, TMR4_EVT_CH, &stcTmr4EventInit);

    /* TMR4 event: set delay object */
    TMR4_OC_SetCompareValue(TMR4_UNIT, TMR4_OC_CH_UH, stcTmr4Init.u16PeriodValue / 4U);
    TMR4_EVT_SetDelayObject(TMR4_UNIT, TMR4_EVT_CH, TMR4_EVT_DELAY_OCCRXH);

    /* Start TMR4 count. */
    TMR4_Start(TMR4_UNIT);

    for (;;) {
        if (SET == ADC_GetStatus(ADC_UNIT, ADC_FLAG_EOCA)) {
            m_u16AdcVal = ADC_GetValue(ADC_UNIT, ADC_CH);
            ADC_ClearStatus(ADC_UNIT, ADC_FLAG_EOCA);

            DDL_Printf("ADC sample value: %d\r\n", m_u16AdcVal);
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
