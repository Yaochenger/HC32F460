/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/test_impl_item/test_impl_adc.c
 * @brief This file provides firmware functions to implement the ADC test.
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
#include "hc32_ll.h"
#include "stl_utility.h"
#include "test_impl_adc.h"

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @defgroup Test_Implement_ADC Test Implement ADC
 * @{
 */

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static __IO uint16_t m_u16AdcSampleValue;
static __IO uint32_t m_u32AdcErrCount = 0UL;
static __IO uint8_t m_u32AdcActived = STL_OFF;

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @defgroup Test_Implement_ADC_Global_Functions Test Implement ADC Global Functions
 * @{
 */

/**
 * @brief  ADC1 AWD0 IRQ callback.
 * @param  None
 * @retval None
 */
static void ADC1_ChCmp_IrqCallback(void)
{
    ADC_AWD_Cmd(STL_ADC_UNIT, STL_ADC_AWD_UNIT, DISABLE);

    m_u32AdcActived = STL_OFF;
    m_u32AdcErrCount++;
    m_u16AdcSampleValue = ADC_GetValue(STL_ADC_UNIT, STL_ADC_CH);
    ADC_AWD_ClearStatus(STL_ADC_UNIT, STL_ADC_AWD_FLAG);
}

/**
 * @brief  Set specified ADC pin to analog mode.
 * @param  [in] u8Port          IO port
 * @param  [in] u16Pin          IO pin
 * @retval None
 */
static void ADC_SetPinAnalogMode(uint8_t u8Port, uint16_t u16Pin)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(u8Port, u16Pin, &stcGpioInit);
}

/**
 * @brief  AWD configuration.
 * @param  [in]  ADCx                   Pointer to ADC instance register base.
 * @param  [in]  u8Ch                   The ADC channel
 * @param  [in]  u16LowThreshold        The low threshold
 * @param  [in]  u16HighThreshold       The high threshold
 * @retval None
 */
static void ADC_AwdConfig(CM_ADC_TypeDef *ADCx, uint8_t u8Ch,
                          uint16_t u16LowThreshold, uint16_t u16HighThreshold)
{
    stc_adc_awd_config_t stcAwd;
    stc_irq_signin_config_t stcIrq;

    stcAwd.u16WatchdogMode  = ADC_AWD_MD_CMP_OUT;
    stcAwd.u16LowThreshold  = u16LowThreshold;
    stcAwd.u16HighThreshold = u16HighThreshold;
    (void)ADC_AWD_Config(ADCx, STL_ADC_AWD_UNIT, u8Ch, &stcAwd);

    stcIrq.enIntSrc    = STL_ADC_AWD_INT_SRC;
    stcIrq.enIRQn      = STL_ADC_AWD_INT_IRQn;
    stcIrq.pfnCallback = &ADC1_ChCmp_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, STL_ADC_AWD_IRQ_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    ADC_AWD_IntCmd(ADCx, STL_ADC_AWD_TYPE, ENABLE);
}

/**
 * @brief  Initializes ADC.
 * @param  [in]  ADCx                   Pointer to ADC instance register base.
 * @param  [in]  u8Ch                   The ADC channel
 * @retval None
 */
static void ADC_InitConfig(CM_ADC_TypeDef *ADCx, uint8_t u8Ch)
{
    stc_adc_init_t stcAdcInit;

    /* Initialize ADC. */
    (void)ADC_StructInit(&stcAdcInit);
    stcAdcInit.u16ScanMode = ADC_MD_SEQA_CONT;
    (void)ADC_Init(ADCx, &stcAdcInit);

    /* Enable ADC channels. */
    ADC_ChCmd(ADCx, ADC_SEQ_A, u8Ch, ENABLE);
}

/**
 * @brief  ADC test initialize in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Initialize successfully.
 *           - STL_ERR:         Initialize unsuccessfully.
 */
uint32_t STL_AdcRuntimeInit(void)
{
    /* Enable ADC peripheral clock. */
    STL_ADC_FCG_ENABLE();

    /* Set the ADC pin to analog input mode. */
    ADC_SetPinAnalogMode(STL_ADC_PORT, STL_ADC_PIN);

    /* Configure ADC channel. */
    ADC_InitConfig(STL_ADC_UNIT, STL_ADC_CH);

    /* Configure ADC window. */
    ADC_AwdConfig(STL_ADC_UNIT, STL_ADC_CH, STL_ADC_WINDOW_LOW, STL_ADC_WINDOW_HIGH);

    /* Start conversion */
    ADC_Start(STL_ADC_UNIT);

    return STL_OK;
}

/**
 * @brief  ADC test in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Test pass.
 *           - STL_ERR:         Test fail.
 */
uint32_t STL_AdcRuntimeTest(void)
{
    uint32_t u32Ret = STL_OK;

    if (m_u32AdcErrCount > 0UL) {
        u32Ret = STL_ERR;
        STL_Printf("ADC sample value: %d out of range(%d, %d) \r\n", m_u16AdcSampleValue, STL_ADC_WINDOW_LOW, STL_ADC_WINDOW_HIGH);
    }

    if (STL_OFF == m_u32AdcActived) {
        m_u32AdcErrCount = 0UL;
        m_u32AdcActived = STL_ON;

        ADC_AWD_Cmd(STL_ADC_UNIT, ADC_AWD0, ENABLE);
    }

    return u32Ret;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
