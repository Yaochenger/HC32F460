/**
 *******************************************************************************
 * @file  adc/adc_channel_remap/source/main.c
 * @brief Main program ADC channel remap for the Device Driver Library.
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
 * @addtogroup ADC_Channel_Remap
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

/* Potentiometer pin PC0, maps to ADC_CH10 by default. */
#define ADC_CH_POTENTIOMETER            (ADC_CH10)
#define ADC_PIN_POTENTIOMETER           (ADC1_PIN_PC0)
#define ADC_CH_PORT                     (GPIO_PORT_C)
#define ADC_CH_PIN                      (GPIO_PIN_00)

/* Potentiometer pin PC0, remap to ADC_CH0. */
#define ADC_REMAP_PIN                   (ADC_PIN_POTENTIOMETER)
#define ADC_REMAP_PIN_DEST_CH           (ADC_CH0)

/* ADC sequence to be used. */
#define ADC_SEQ                         (ADC_SEQ_A)
/* Flag of conversion end. */
#define ADC_EOC_FLAG                    (ADC_FLAG_EOCA)

/* ADC reference voltage. The voltage of pin VREFH. */
#define ADC_VREF                        (3.3F)

/* ADC accuracy(according to the resolution of ADC). */
#define ADC_ACCURACY                    (1UL << 12U)

/* Calculate the voltage(mV). */
#define ADC_CAL_VOL(adcVal)             (uint16_t)((((float32_t)(adcVal) * ADC_VREF) / ((float32_t)ADC_ACCURACY)) * 1000.F)

/* Timeout value. */
#define ADC_TIMEOUT_VAL                 (1000U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void AdcConfig(void);
static void AdcInitConfig(void);
static void AdcSetPinAnalogMode(void);
static void AdcPolling(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of adc_channel_remap project
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

    /***************** Configuration end, application start **************/

    for (;;) {
        AdcPolling();
        DDL_DelayMS(500UL);
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

    /* 2. Modify the default value depends on the application. Not needed here. */
    (void)ADC_StructInit(&stcAdcInit);

    /* 3. Initializes ADC. */
    (void)ADC_Init(ADC_UNIT, &stcAdcInit);

    /* 4. ADC channel configuration. */
    /* 4.1 Set the ADC pin to analog input mode. */
    AdcSetPinAnalogMode();
    /* 4.2 Chnannel remap */
    ADC_ChRemap(ADC_UNIT, ADC_REMAP_PIN_DEST_CH, ADC_REMAP_PIN);
    /* 4.3 Enable ADC channels. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_REMAP_PIN_DEST_CH, ENABLE);
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
    (void)GPIO_Init(ADC_CH_PORT, ADC_CH_PIN, &stcGpioInit);
}

/**
 * @brief  Use ADC in polling mode.
 * @param  None
 * @retval None
 */
static void AdcPolling(void)
{
    uint16_t u16AdcValue;
    int32_t iRet = LL_ERR;
    __IO uint32_t u32TimeCount = 0UL;

    /* Can ONLY start sequence A conversion.
       Sequence B needs hardware trigger to start conversion. */
    ADC_Start(ADC_UNIT);
    do {
        if (ADC_GetStatus(ADC_UNIT, ADC_EOC_FLAG) == SET) {
            ADC_ClearStatus(ADC_UNIT, ADC_EOC_FLAG);
            iRet = LL_OK;
            break;
        }
    } while (u32TimeCount++ < ADC_TIMEOUT_VAL);

    if (iRet == LL_OK) {
        /* Get any ADC value of sequence A channel that needed. */
        u16AdcValue = ADC_GetValue(ADC_UNIT, ADC_REMAP_PIN_DEST_CH);
        DDL_Printf("The ADC value of potentiometer is %u, voltage is %u mV\r\n",
                   u16AdcValue, ADC_CAL_VOL(u16AdcValue));
    } else {
        ADC_Stop(ADC_UNIT);
        DDL_Printf("ADC exception.\r\n");
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
