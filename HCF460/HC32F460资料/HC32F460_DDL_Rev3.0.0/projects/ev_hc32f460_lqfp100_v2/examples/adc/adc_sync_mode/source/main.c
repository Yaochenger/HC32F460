/**
 *******************************************************************************
 * @file  adc/adc_sync_mode/source/main.c
 * @brief Main program ADC sync mode for the Device Driver Library.
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
 * @addtogroup ADC_Sync_Mode
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* ADC synchronous mode. @ref ADC_Sync_Mode */
#define ADC_SYNC_MD                     (ADC_SYNC_SINGLE_DELAY_TRIG)

#if (ADC_SYNC_MD == ADC_SYNC_SINGLE_DELAY_TRIG)
#define ADC_SYNC_SPL_TIME               (11U)
#define ADC_SYNC_TRIG_DELAY             (12U)
#define ADC_MD                          (ADC_MD_SEQA_CONT)

#elif (ADC_SYNC_MD == ADC_SYNC_SINGLE_PARALLEL_TRIG)
#define ADC_SYNC_SPL_TIME               (11U)
#define ADC_SYNC_TRIG_DELAY             (0U)
#define ADC_MD                          (ADC_MD_SEQA_CONT)

#elif (ADC_SYNC_MD == ADC_SYNC_CYCLIC_DELAY_TRIG)
#define ADC_SYNC_SPL_TIME               (11U)
#define ADC_SYNC_TRIG_DELAY             (30U)
#define ADC_MD                          (ADC_MD_SEQA_SINGLESHOT)

#elif (ADC_SYNC_MD == ADC_SYNC_CYCLIC_PARALLEL_TRIG)
#define ADC_SYNC_SPL_TIME               (11U)
#define ADC_SYNC_TRIG_DELAY             (120U)
#define ADC_MD                          (ADC_MD_SEQA_SINGLESHOT)

#else
#error "This sync mode is NOT supported."

#endif /* #if (ADC_SYNC_MD == ADC_SYNC_SINGLE_DELAY_TRIG) */

/* ADC peripheral clock for this example. */
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1 | FCG3_PERIPH_ADC2)

/*
 * ADC channels definition for this example.
 * NOTE!!! DO NOT enable sequence B, it will disturb the synchronization timing.
 */
#if (ADC_SYNC_MD == ADC_SYNC_SINGLE_DELAY_TRIG)
/* These definitions just for a sampling rate of 5Msps. */
#define ADC1_CH                         (ADC_CH4)
#define ADC2_CH                         (ADC_CH0)
#define ADC_CH_PORT                     (GPIO_PORT_A)
#define ADC_CH_PIN                      (GPIO_PIN_04)

#else
#define ADC1_CHX                        (ADC_CH0)
#define ADC1_CHX_PORT                   (GPIO_PORT_A)
#define ADC1_CHX_PIN                    (GPIO_PIN_00)

#define ADC1_CHY                        (ADC_CH1)
#define ADC1_CHY_PORT                   (GPIO_PORT_A)
#define ADC1_CHY_PIN                    (GPIO_PIN_01)

#define ADC2_CHX                        (ADC_CH6)
#define ADC2_CHX_PORT                   (GPIO_PORT_C)
#define ADC2_CHX_PIN                    (GPIO_PIN_00)

#define ADC2_CHY                        (ADC_CH7)
#define ADC2_CHY_PORT                   (GPIO_PORT_C)
#define ADC2_CHY_PIN                    (GPIO_PIN_01)
#endif

/* ADC channel sampling time */
#define ADC_SPL_TIME                    ADC_SYNC_SPL_TIME

/* Hard trigger */
#define ADC_SEQ_HARDTRIG                (ADC_HARDTRIG_ADTRG_PIN)
#define ADC_SEQ_TRIG_PORT               (GPIO_PORT_E)
#define ADC_SEQ_TRIG_PIN                (GPIO_PIN_07)
#define ADC_SEQ_TRIG_PIN_FUNC           (GPIO_FUNC_1)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void AdcConfig(void);
static void AdcClockConfig(void);
static void AdcInitConfig(void);
static void AdcSetPinAnalogMode(void);
static void AdcSyncConfig(void);
static void AdcHardTriggerConfig(void);

static void IndicateConfig(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of adc_sync_mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);
    /* Configures the system clock to 200MHz. */
    BSP_CLK_Init();
    /* Configures ADC. */
    AdcConfig();
    /* Use event port to indicate the scan timing of ADC synchronous mode. */
    IndicateConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /***************** Configuration end, application start **************/

    for (;;) {
    }
}

/**
 * @brief  ADC configuration.
 * @param  None
 * @retval None
 */
static void AdcConfig(void)
{
    AdcClockConfig();
    AdcInitConfig();
    AdcSyncConfig();
    AdcHardTriggerConfig();
}

/**
 * @brief  Configures ADC clock.
 * @param  None
 * @retval None
 */
static void AdcClockConfig(void)
{
    /*
     * 1. Configures UPLL and the divider of UPLLx(x=P, Q, R).
     *    UPLLx(x=P, Q, R) is used as both the digital interface clock and the analog circuit clock.
     *    UPLLx(x=P, Q, R) must be in range [1MHz, 60MHz] for ADC use.
     *    The input source of UPLL is XTAL(8MHz).
     */
    stc_clock_pllx_init_t stcUpllInit;

    /* UPLL and MPLL share the same clock source. The clock source(XTAL) is configured in BSP_CLK_Init(). */

    /* UPLL config */
    (void)CLK_PLLxStructInit(&stcUpllInit);
    /*
     * UPLLx(x=P, Q, R) = ((PLL_source / PLLM) * PLLN) / PLLx
     * UPLLP = (8 / 2) * 60 / 4 = 60MHz
     * UPLLQ = (8 / 2) * 60 / 4 = 60MHz
     * UPLLR = (8 / 2) * 60 / 4 = 60MHz
     */
    stcUpllInit.u8PLLState = CLK_PLLX_ON;
    stcUpllInit.PLLCFGR    = 0UL;
    stcUpllInit.PLLCFGR_f.PLLM = (2UL  - 1UL);
    stcUpllInit.PLLCFGR_f.PLLN = (60UL - 1UL);
    stcUpllInit.PLLCFGR_f.PLLR = (4UL  - 1UL);
    stcUpllInit.PLLCFGR_f.PLLQ = (4UL  - 1UL);
    stcUpllInit.PLLCFGR_f.PLLP = (4UL  - 1UL);
    (void)CLK_PLLxInit(&stcUpllInit);

    /* 2. Specifies the clock source of ADC. */
    CLK_SetPeriClockSrc(CLK_PERIPHCLK_PLLXP);
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
    stcAdcInit.u16ScanMode = ADC_MD;

    /* 3. Initializes ADC. */
    (void)ADC_Init(CM_ADC1, &stcAdcInit);
    (void)ADC_Init(CM_ADC2, &stcAdcInit);

    /* 4. ADC channel configuration. */
    /* 4.1 Set the ADC pin to analog input mode. */
    AdcSetPinAnalogMode();

#if (ADC_SYNC_MD == ADC_SYNC_SINGLE_DELAY_TRIG)
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(CM_ADC1, ADC_SEQ_A, ADC1_CH, ENABLE);
    ADC_ChCmd(CM_ADC2, ADC_SEQ_A, ADC2_CH, ENABLE);
    /* 4.3 Set sample time */
    ADC_SetSampleTime(CM_ADC1, ADC1_CH, ADC_SPL_TIME);
    ADC_SetSampleTime(CM_ADC2, ADC2_CH, ADC_SPL_TIME);
#else
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(CM_ADC1, ADC_SEQ_A, ADC1_CHX, ENABLE);
    ADC_ChCmd(CM_ADC1, ADC_SEQ_A, ADC1_CHY, ENABLE);
    ADC_ChCmd(CM_ADC2, ADC_SEQ_A, ADC2_CHX, ENABLE);
    ADC_ChCmd(CM_ADC2, ADC_SEQ_A, ADC2_CHY, ENABLE);
    /* 4.3 Set sample time */
    ADC_SetSampleTime(CM_ADC1, ADC1_CHX, ADC_SPL_TIME);
    ADC_SetSampleTime(CM_ADC1, ADC1_CHY, ADC_SPL_TIME);
    ADC_SetSampleTime(CM_ADC2, ADC2_CHX, ADC_SPL_TIME);
    ADC_SetSampleTime(CM_ADC2, ADC2_CHY, ADC_SPL_TIME);
#endif
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
#if (ADC_SYNC_MD == ADC_SYNC_SINGLE_DELAY_TRIG)
    (void)GPIO_Init(ADC_CH_PORT, ADC_CH_PIN, &stcGpioInit);
#else
    (void)GPIO_Init(ADC1_CHX_PORT, ADC1_CHX_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC1_CHY_PORT, ADC1_CHY_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC2_CHX_PORT, ADC2_CHX_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC2_CHY_PORT, ADC2_CHY_PIN, &stcGpioInit);
#endif
}

/**
 * @brief  Synchronous mode configuration.
 * @param  None
 * @retval None
 */
static void AdcSyncConfig(void)
{
    ADC_SyncModeConfig(ADC_SYNC_ADC1_ADC2, ADC_SYNC_MD, ADC_SYNC_TRIG_DELAY);
    ADC_SyncModeCmd(ENABLE);
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
    ADC_TriggerConfig(CM_ADC1, ADC_SEQ_A, ADC_SEQ_HARDTRIG);
    ADC_TriggerCmd(CM_ADC1, ADC_SEQ_A, ENABLE);
}

/**
 * @brief  Use event port 2.5 and 4.8 to indicate the scan timing of ADC synchronous mode.
 *         ADC1: PB5
 *         ADC2: PD8
 * @param  None
 * @retval None
 */
static void IndicateConfig(void)
{
    stc_ep_init_t stcEpInit;

    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    /* Set PB5 as event port 2.5. */
    GPIO_SetFunc(GPIO_PORT_B, GPIO_PIN_05, GPIO_FUNC_14);
    /* Set PD8 as event port 4.8. */
    GPIO_SetFunc(GPIO_PORT_D, GPIO_PIN_08, GPIO_FUNC_14);

    /* Set event port 2.5 and 4.8 as output function. */
    EP_StructInit(&stcEpInit);
    stcEpInit.u32PinDir        = EP_DIR_OUT;
    stcEpInit.enPinState       = EVT_PIN_RESET;
    stcEpInit.u32PinTriggerOps = EP_OPS_TOGGLE;
    EP_Init(EVT_PORT_2, EVT_PIN_05, &stcEpInit);
    EP_Init(EVT_PORT_4, EVT_PIN_08, &stcEpInit);

    /* Set EVT_SRC_ADC1_EOCA as the trigger source for event port 2. */
    AOS_SetTriggerEventSrc(AOS_EVTPORT12, EVT_SRC_ADC1_EOCA);
    /* Set EVT_SRC_ADC2_EOCA as the trigger source for event port 4. */
    AOS_SetTriggerEventSrc(AOS_EVTPORT34, EVT_SRC_ADC2_EOCA);
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
