/**
 *******************************************************************************
 * @file  adc/adc_base/source/main.c
 * @brief Main program ADC base for the Device Driver Library.
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
 * @addtogroup ADC_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/* The clock source of ADC. */
#define ADC_CLK_SYS_CLK                 (1U)
#define ADC_CLK_MPLL                    (2U)
#define ADC_CLK_UPLL                    (3U)

/*
 * Selects a clock source according to the application requirements.
 * PCLK4 is the clock for digital interface.
 * PCLK2 is the clock for analog circuit.
 * PCLK4 and PCLK2 are synchronous when the clock source is xPLL.
 * PCLK4 : PCLK2 = 1:1, 2:1, 4:1, 8:1, 1:2, 1:4.
 * PCLK2 is in range [1MHz, 60MHz].
 * If the system clock is selected as the ADC clock, macro 'ADC_ADC_CLK' can only be defined as 'CLK_PERIPHCLK_PCLK'.
 * If MPLL is selected as the ADC clock, macro 'ADC_ADC_CLK' can be defined as 'CLK_PERIPHCLK_PLLx'(x=P, Q, R).
 * If UPLL is selected as the ADC clock, macro 'ADC_ADC_CLK' can be defined as 'CLK_PERIPHCLK_UPLLx'(x=P, Q, R).
 */
#define ADC_CLK_SEL                     (ADC_CLK_SYS_CLK)

#if (ADC_CLK_SEL == ADC_CLK_SYS_CLK)
#define ADC_CLK                         (CLK_PERIPHCLK_PCLK)

#elif (ADC_CLK_SEL == ADC_CLK_MPLL)
#define ADC_CLK                         (CLK_PERIPHCLK_PLLQ)

#elif (ADC_CLK_SEL == ADC_CLK_UPLL)
#define ADC_CLK                         (CLK_PERIPHCLK_PLLXP)

#else
#error "The clock source your selected does not exist!!!"
#endif

/* ADC unit instance for this example. */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1)

/* Selects ADC channels that needed. */
#define ADC_CH_POTENTIOMETER            (ADC_CH10)
#define ADC_CH                          (ADC_CH_POTENTIOMETER)
#define ADC_CH_PORT                     (GPIO_PORT_C)
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
static void AdcClockConfig(void);
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
 * @brief  Main function of adc_base project
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
    AdcClockConfig();
    AdcInitConfig();
}

/**
 * @brief  Configures ADC clock.
 * @param  None
 * @retval None
 */
static void AdcClockConfig(void)
{
#if (ADC_CLK_SEL == ADC_CLK_SYS_CLK)
    /*
     * 1. Configures the clock divider of PCLK2 and PCLK4 here or in the function of configuring the system clock.
     *    In this example, the system clock is MRC@8MHz.
     *    PCLK4 is the digital interface clock, and PCLK2 is the analog circuit clock.
     *    Make sure that PCLK2 and PCLK4 meet the following conditions:
     *      PCLK4 : PCLK2 = 1:1, 2:1, 4:1, 8:1, 1:2, 1:4.
     *      PCLK2 is in range [1MHz, 60MHz].
     */
    CLK_SetClockDiv((CLK_BUS_PCLK2 | CLK_BUS_PCLK4), (CLK_PCLK2_DIV8 | CLK_PCLK4_DIV2));

#elif (ADC_CLK_SEL == ADC_CLK_MPLL)
    /*
     * 1. Configures MPLL and the divider of MPLLx(x=P, Q, R).
     *    MPLLx(x=P, Q, R) is used as both the digital interface clock and the analog circuit clock.
     *    MPLLx(x=P, Q, R) must be in range [1MHz, 60MHz] for ADC use.
     *    The input source of MPLL is XTAL(8MHz).
     */
    stc_clock_pll_init_t stcMpllInit;
    stc_clock_xtal_init_t stcXtalInit;

    /* Configures XTAL. MPLL input source is XTAL. */
    (void)CLK_XtalStructInit(&stcXtalInit);
    stcXtalInit.u8State      = CLK_XTAL_ON;
    stcXtalInit.u8Drv        = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8Mode       = CLK_XTAL_MD_OSC;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_499US;
    (void)CLK_XtalInit(&stcXtalInit);

    (void)CLK_PLLStructInit(&stcMpllInit);
    /*
     * MPLLx(x=P, Q, R) = ((PLL_source / PLLM) * PLLN) / PLLx
     * MPLLP = (8 / 1) * 40 /8  = 40MHz
     * MPLLQ = (8 / 1) * 40 /10 = 32MHz
     * MPLLR = (8 / 1) * 40 /16 = 20MHz
     */
    stcMpllInit.u8PLLState = CLK_PLL_ON;
    stcMpllInit.PLLCFGR = 0UL;
    stcMpllInit.PLLCFGR_f.PLLM = (1UL  - 1UL);
    stcMpllInit.PLLCFGR_f.PLLN = (40UL - 1UL);
    stcMpllInit.PLLCFGR_f.PLLP = (8UL  - 1UL);
    stcMpllInit.PLLCFGR_f.PLLQ = (10UL - 1UL);
    stcMpllInit.PLLCFGR_f.PLLR = (16UL - 1UL);
    /* stcMpllInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL; */
    (void)CLK_PLLInit(&stcMpllInit);

#elif (ADC_CLK_SEL == ADC_CLK_UPLL)
    /*
     * 1. Configures UPLL and the divider of UPLLx(x=P, Q, R).
     *    UPLLx(x=P, Q, R) is used as both the digital interface clock and the analog circuit clock.
     *    UPLLx(x=P, Q, R) must be in range [1MHz, 60MHz] for ADC use.
     *    The input source of UPLL is HRC(16MHz).
     */
    stc_clock_pllx_init_t stcUpllInit;

    /* Enable HRC(16MHz) for UPLL. */
    CLK_HrcCmd(ENABLE);

    /* Specify the input source of UPLL. NOTE!!! UPLL and MPLL use the same input source. */
    CLK_SetPLLSrc(CLK_PLL_SRC_HRC);
    /* UPLL configuration. */
    (void)CLK_PLLxStructInit(&stcUpllInit);
    /*
     * UPLLx(x=P, Q, R) = ((PLL_source / PLLM) * PLLN) / PLLx
     * UPLLP = (16 / 2) * 40 / 8  = 40MHz
     * UPLLQ = (16 / 2) * 40 / 10 = 32MHz
     * UPLLR = (16 / 2) * 40 / 16 = 20MHz
     */
    stcUpllInit.u8PLLState = CLK_PLLX_ON;
    stcUpllInit.PLLCFGR = 0UL;
    stcUpllInit.PLLCFGR_f.PLLM = (2UL  - 1UL);
    stcUpllInit.PLLCFGR_f.PLLN = (40UL - 1UL);
    stcUpllInit.PLLCFGR_f.PLLR = (8UL  - 1UL);
    stcUpllInit.PLLCFGR_f.PLLQ = (10UL - 1UL);
    stcUpllInit.PLLCFGR_f.PLLP = (16UL - 1UL);
    (void)CLK_PLLxInit(&stcUpllInit);
#endif
    /* 2. Specifies the clock source of ADC. */
    CLK_SetPeriClockSrc(ADC_CLK);
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
        u16AdcValue = ADC_GetValue(ADC_UNIT, ADC_CH);
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
