/**
 *******************************************************************************
 * @file  adc/adc_extended_channel/source/main.c
 * @brief Main program ADC extended channel for the Device Driver Library.
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
 * @addtogroup ADC_Extended_Channel
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Internal analog source definition. */
#define ADC_INTERN_SRC_REF_VOL          (1U)
#define ADC_INTERN_SRC_DAC              (2U)
/* Select an internal analog source. */
#define ADC_INTERN_SRC_SEL              (ADC_INTERN_SRC_DAC)

/* ADC unit instance for this example. */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1)

/* Selects ADC channels that needed. */
#define ADC_CH                          (ADC1_EXT_CH)

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

/* Calculate 8bit-DAC value. */
#define ADC_CAL_VAL(vol)                ((uint16_t)(((float32_t)(vol) * (float32_t)(1UL << 8U)) / (float32_t)ADC_VREF))
/* Specifies the output voltage of 8bit-DAC. */
#define DAC_VAL                         ADC_CAL_VAL(2.2F)

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
static void AdcInternalSrcConfig(void);
static void AdcPolling(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of adc_extended_channel project
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
    AdcInternalSrcConfig();
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
    /* 4.1 Set the ADC pin to analog input mode, not needed here. */
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_CH, ENABLE);

    /* 5. Conversion data average calculation function, if needed.
          Call ADC_ConvDataAverageChCmd() again to enable more average channels if needed. */
    ADC_ConvDataAverageConfig(ADC_UNIT, ADC_AVG_CNT8);
    ADC_ConvDataAverageChCmd(ADC_UNIT, ADC_CH, ENABLE);
}

/**
 * @brief  Configures ADC internal analog source.
 * @param  None
 * @retval None
 */
static void AdcInternalSrcConfig(void)
{
#if (ADC_INTERN_SRC_SEL == ADC_INTERN_SRC_REF_VOL)
    /* Configure PWR monitor and enable it. */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_CMP, ENABLE);
    PWC_PowerMonitorCmd(ENABLE);
    CMP_8BitDAC_AdcRefCmd(CMP_ADC_REF_VREF, ENABLE);
#elif (ADC_INTERN_SRC_SEL == ADC_INTERN_SRC_DAC)
    /* Enable DAC to ADC. */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_CMP, ENABLE);
    CMP_8BitDAC_WriteData(CMP_8BITDAC_CH1, DAC_VAL);
    CMP_8BitDAC_AdcRefCmd(CMP_ADC_REF_DA1, ENABLE);
    CMP_8BitDAC_Cmd(CMP_8BITDAC_CH1, ENABLE);
#endif
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
        u16AdcValue = ADC_GetValue(ADC_UNIT, ADC_CH);
#if (ADC_INTERN_SRC_SEL == ADC_INTERN_SRC_REF_VOL)
        DDL_Printf("Internal reference voltage is %u mV\r\n", ADC_CAL_VOL(u16AdcValue));
#elif (ADC_INTERN_SRC_SEL == ADC_INTERN_SRC_DAC)
        DDL_Printf("DAC output voltage is %u mV\r\n", ADC_CAL_VOL(u16AdcValue));
#endif
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
