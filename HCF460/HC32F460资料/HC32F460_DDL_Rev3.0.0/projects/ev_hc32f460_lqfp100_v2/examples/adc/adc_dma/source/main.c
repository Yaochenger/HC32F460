/**
 *******************************************************************************
 * @file  adc/adc_dma/source/main.c
 * @brief Main program ADC DMA for the Device Driver Library.
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
 * @addtogroup ADC_DMA
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
#define ADC_CHX                         (ADC_CH2)
#define ADC_CHX_PORT                    (GPIO_PORT_A)
#define ADC_CHX_PIN                     (GPIO_PIN_02)

#define ADC_CHY                         (ADC_CH3)
#define ADC_CHY_PORT                    (GPIO_PORT_A)
#define ADC_CHY_PIN                     (GPIO_PIN_03)

#define ADC_CHZ                         (ADC_CH10)
#define ADC_CHZ_PORT                    (GPIO_PORT_C)
#define ADC_CHZ_PIN                     (GPIO_PIN_00)

#define ADC_CH_MIN                      (ADC_CHX)
#define ADC_CH_MAX                      (ADC_CHZ)
#define ADC_DR_START                    ((uint32_t)&ADC_UNIT->DR2)
#define PTTM_VAL_IDX                    (8U)

/* ADC sequence to be used. */
#define ADC_SEQ                         (ADC_SEQ_A)

/* Hard trigger of the specified sequence. */
#define ADC_SEQ_HARDTRIG                (ADC_HARDTRIG_ADTRG_PIN)
#define ADC_SEQ_TRIG_PORT               (GPIO_PORT_E)
#define ADC_SEQ_TRIG_PIN                (GPIO_PIN_07)
#define ADC_SEQ_TRIG_PIN_FUNC           (GPIO_FUNC_1)

/*
 * Definitions of DMA.
 * 'APP_DMA_BLOCK_SIZE': 1~1024, inclusive. 1~16 for ADC1 and ADC2; 1~20 for ADC3.
 * 'APP_DMA_TRANS_COUNT': 0~65535, inclusive. 0: always transmit.
 */
#define DMA_UNIT                        (CM_DMA1)
#define DMA_PERIPH_CLK                  (FCG0_PERIPH_DMA1)
#define DMA_CH                          (DMA_CH0)
#define DMA_AOS_TRIG_SEL                (AOS_DMA1_0)

#define DMA_TRANS_CNT                   (0U)
#define DMA_BLOCK_SIZE                  (ADC_CH_MAX - ADC_CH_MIN + 1U)
#define DMA_DATA_WIDTH                  (DMA_DATAWIDTH_16BIT)
#define DMA_SRC_ADDR                    ADC_DR_START
#define DMA_DEST_ADDR                   ((uint32_t)(&m_au16AdcValue[0U]))

#define DMA_TRIG_EVT                    (EVT_SRC_ADC1_EOCA)

#define DMA_INT_TYPE                    (DMA_INT_BTC_CH0)
#define DMA_INT_SRC                     (INT_SRC_DMA1_BTC0)
#define DMA_INT_IRQn                    (INT038_IRQn)
#define DMA_INT_PRIO                    (DDL_IRQ_PRIO_03)
#define DMA_INT_FLAG                    (DMA_FLAG_BTC_CH0)

/* ADC reference voltage. The voltage of pin VREFH. */
#define ADC_VREF                        (3.3F)

/* ADC accuracy(according to the resolution of ADC). */
#define ADC_ACCURACY                    (1UL << 12U)

/* Calculate the voltage(mV). */
#define ADC_CAL_VOL(adcVal)             (uint16_t)((((float32_t)(adcVal) * ADC_VREF) / ((float32_t)ADC_ACCURACY)) * 1000.F)

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

static void DmaConfig(void);
static void DmaIrqConfig(void);
static void DMA_IrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint16_t m_au16AdcValue[DMA_BLOCK_SIZE];
__IO static uint8_t m_u8AdcValUpdated = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of adc_dma project
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
        /* Make a falling edge on the specified pin ADTRG to start ADC. */
        if (m_u8AdcValUpdated != 0U) {
            m_u8AdcValUpdated = 0U;
            DDL_Printf("Sequence A DMA transmission completed.\r\n");
            /* User code: Use the ADC value of sequence A. */
            DDL_Printf("The ADC value of potentiometer is %u, voltage is %u mV\r\n", \
                       m_au16AdcValue[PTTM_VAL_IDX], ADC_CAL_VOL(m_au16AdcValue[PTTM_VAL_IDX]));
        }
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
    DmaConfig();
    AdcHardTriggerConfig();
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

    /* 3. Initializes ADC. */
    (void)ADC_Init(ADC_UNIT, &stcAdcInit);

    /* 4. ADC channel configuration. */
    /* 4.1 Set the ADC pin to analog input mode. */
    AdcSetPinAnalogMode();
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_CHX, ENABLE);
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_CHY, ENABLE);
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_CHZ, ENABLE);
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
    (void)GPIO_Init(ADC_CHX_PORT, ADC_CHX_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC_CHY_PORT, ADC_CHY_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC_CHZ_PORT, ADC_CHZ_PIN, &stcGpioInit);
}

/**
 * @brief  ADC hard trigger configuration.
 * @param  None
 * @retval None
 */
static void AdcHardTriggerConfig(void)
{
    /************** Hard trigger of sequence A ****************/
    GPIO_SetFunc(ADC_SEQ_TRIG_PORT, ADC_SEQ_TRIG_PIN, ADC_SEQ_TRIG_PIN_FUNC);
    ADC_TriggerConfig(ADC_UNIT, ADC_SEQ, ADC_SEQ_HARDTRIG);
    ADC_TriggerCmd(ADC_UNIT, ADC_SEQ, ENABLE);
}

/**
 * @brief  DMA configuration.
 * @param  None
 * @retval None
 */
static void DmaConfig(void)
{
    stc_dma_init_t stcDmaInit;
    stc_dma_repeat_init_t stcDmaRptInit;

    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32IntEn       = DMA_INT_ENABLE;
    stcDmaInit.u32SrcAddr     = DMA_SRC_ADDR;
    stcDmaInit.u32DestAddr    = DMA_DEST_ADDR;
    stcDmaInit.u32DataWidth   = DMA_DATA_WIDTH;
    stcDmaInit.u32BlockSize   = DMA_BLOCK_SIZE;
    stcDmaInit.u32TransCount  = DMA_TRANS_CNT;
    stcDmaInit.u32SrcAddrInc  = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;

    /* Enable DMA peripheral clock and AOS function. */
    FCG_Fcg0PeriphClockCmd(DMA_PERIPH_CLK, ENABLE);
    (void)DMA_Init(DMA_UNIT, DMA_CH, &stcDmaInit);

    stcDmaRptInit.u32Mode      = DMA_RPT_BOTH;
    stcDmaRptInit.u32SrcCount  = DMA_BLOCK_SIZE;
    stcDmaRptInit.u32DestCount = DMA_BLOCK_SIZE;
    (void)DMA_RepeatInit(DMA_UNIT, DMA_CH, &stcDmaRptInit);

    /* Enable AOS clock */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    /* Set DMA trigger source */
    AOS_SetTriggerEventSrc(DMA_AOS_TRIG_SEL, DMA_TRIG_EVT);

    /* DMA IRQ configuration. */
    DmaIrqConfig();

    DMA_Cmd(DMA_UNIT, ENABLE);
    DMA_ChCmd(DMA_UNIT, DMA_CH, ENABLE);
}

/**
 * @brief  DMA interrupt configuration.
 * @param  None
 * @retval None
 */
static void DmaIrqConfig(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;

    stcIrqSignConfig.enIntSrc    = DMA_INT_SRC;
    stcIrqSignConfig.enIRQn      = DMA_INT_IRQn;
    stcIrqSignConfig.pfnCallback = &DMA_IrqCallback;

    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    DMA_ClearTransCompleteStatus(DMA_UNIT, DMA_INT_FLAG);

    /* NVIC setting */
    NVIC_ClearPendingIRQ(DMA_INT_IRQn);
    NVIC_SetPriority(DMA_INT_IRQn, DMA_INT_PRIO);
    NVIC_EnableIRQ(DMA_INT_IRQn);
}

/**
 * @brief  DMA IRQ handler.
 * @param  None
 * @retval None
 */
static void DMA_IrqCallback(void)
{
    DMA_ClearTransCompleteStatus(DMA_UNIT, DMA_INT_FLAG);
    m_u8AdcValUpdated = 1U;
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
