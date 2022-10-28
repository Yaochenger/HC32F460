/**
 *******************************************************************************
 * @file  adc/adc_pga/source/main.c
 * @brief Main program ADC PGA for the Device Driver Library.
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
 * @addtogroup ADC_PGA
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

/* Definitions of PGA. */
#define ADC_PGA_PIN                     (ADC_PGA_PIN_ADC1_PA0)
#define ADC_PGA_GAIN                    (ADC_PGA_GAIN_2P286)
#define ADC_PGA_VSS                     (ADC_PGA_VSS_AVSS)

/* The default ADC channel of PGA pin. */
#define ADC_CH_PGA                      (ADC_CH0)
#define ADC_CH                          (ADC_CH_PGA)
#define ADC_CH_PORT                     (GPIO_PORT_A)
#define ADC_CH_PIN                      (GPIO_PIN_00)

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
static void AdcPgaConfig(void);
static int32_t AdcPolling(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint16_t m_u16AdcValueBeforePga;
static uint16_t m_u16AdcValueAfterPga;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of adc_pga project
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
        ADC_PGA_Cmd(ADC_UNIT, ADC_PGA1, DISABLE);
        if (AdcPolling() == LL_OK) {
            m_u16AdcValueBeforePga = ADC_GetValue(ADC_UNIT, ADC_CH);
        }

        ADC_PGA_Cmd(ADC_UNIT, ADC_PGA1, ENABLE);
        if (AdcPolling() == LL_OK) {
            m_u16AdcValueAfterPga = ADC_GetValue(ADC_UNIT, ADC_CH);
        }

        DDL_Printf("\t-------------------------------------------------------------------------------------\r\n");
        DDL_Printf("\t--->> PGA1 input voltage is %u mV, output voltage is %u mV\r\n", \
                   ADC_CAL_VOL(m_u16AdcValueBeforePga), ADC_CAL_VOL(m_u16AdcValueAfterPga));
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
    AdcPgaConfig();
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
    /* 4.2 Enable ADC channels. Call ADC_ChCmd() again to enable more channels if needed. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_CH, ENABLE);

    /* 5. Conversion data average calculation function, if needed.
          Call ADC_ConvDataAverageChCmd() again to enable more average channels if needed. */
    ADC_ConvDataAverageConfig(ADC_UNIT, ADC_AVG_CNT8);
    ADC_ConvDataAverageChCmd(ADC_UNIT, ADC_CH, ENABLE);
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
 * @brief  ADC PGA configuration.
 * @param  None
 * @retval None
 */
static void AdcPgaConfig(void)
{
    /* 1. Specify the gain factor and the VSS. */
    ADC_PGA_Config(ADC_UNIT, ADC_PGA1, ADC_PGA_GAIN, ADC_PGA_VSS);
    /* 2. Select PGA input pin. */
    ADC_PGA_SelectInputSrc(ADC_UNIT, ADC_PGA_PIN);
    /* 3. Enable the PGA. */
    ADC_PGA_Cmd(ADC_UNIT, ADC_PGA1, ENABLE);
}

/**
 * @brief  Use ADC in polling mode.
 * @param  [in]  u8Ch                   ADC channel.
 *                                      This parameter can be a value of @ref ADC_Channel
 * @retval int32_t:
 *           - LL_OK:                   No error occurred.
 *           - LL_ERR:                  ADC exception.
 */
static int32_t AdcPolling(void)
{
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

    if (iRet == LL_ERR) {
        /* ADC exception. */
        ADC_Stop(ADC_UNIT);
    }

    return iRet;
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
