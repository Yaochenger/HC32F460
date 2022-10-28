/**
 *******************************************************************************
 * @file  rtc/rtc_calibration_output/source/main.c
 * @brief Main program of RTC Calibration Output for the Device Driver Library.
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
 * @addtogroup RTC_Calibration_Output
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)
/* RTC 1Hz output Port/Pin definition */
#define RTC_ONEHZ_OUTPUT_PORT           (GPIO_PORT_C)
#define RTC_ONEHZ_OUTPUT_PIN            (GPIO_PIN_13)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8SecIntFlag = 0U;
static int16_t u16CompenVal = 0x20;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  RTC period interrupt callback function.
 * @param  None
 * @retval None
 */
static void RTC_Period_IrqCallback(void)
{
    u8SecIntFlag = 1U;
}



/**
 * @brief  RTC configuration.
 * @param  None
 * @retval None
 */
static void RTC_Config(void)
{
    int32_t i32Ret;
    stc_rtc_init_t stcRtcInit;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Configuration RTC output pin */
    GPIO_SetFunc(RTC_ONEHZ_OUTPUT_PORT, RTC_ONEHZ_OUTPUT_PIN, GPIO_FUNC_1);

    /* Reset RTC counter */
    if (LL_ERR_TIMEOUT == RTC_DeInit()) {
        DDL_Printf("Reset RTC failed!\r\n");
    } else {
        /* Configure structure initialization */
        (void)RTC_StructInit(&stcRtcInit);

        /* Configuration RTC structure */
        stcRtcInit.u8ClockSrc     = RTC_CLK_SRC_XTAL32;
        stcRtcInit.u8HourFormat   = RTC_HOUR_FMT_24H;
        stcRtcInit.u8IntPeriod    = RTC_INT_PERIOD_PER_SEC;
        stcRtcInit.u8ClockCompen  = RTC_CLK_COMPEN_ENABLE;
        stcRtcInit.u16CompenValue = (uint16_t)u16CompenVal;
        stcRtcInit.u8CompenMode   = RTC_CLK_COMPEN_MD_UNIFORM;
        (void)RTC_Init(&stcRtcInit);

        /* RTC period interrupt configure */
        stcIrqSignConfig.enIntSrc    = INT_SRC_RTC_PRD;
        stcIrqSignConfig.enIRQn      = INT002_IRQn;
        stcIrqSignConfig.pfnCallback = &RTC_Period_IrqCallback;
        (void)INTC_IrqSignOut(stcIrqSignConfig.enIRQn);
        i32Ret = INTC_IrqSignIn(&stcIrqSignConfig);
        if (LL_OK != i32Ret) {
            /* check parameter */
            for (;;) {
            }
        }

        /* Clear pending */
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        /* Set priority */
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        /* Enable NVIC */
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        /* Enable 1HZ output */
        RTC_OneHzOutputCmd(ENABLE);
        /* Enable period interrupt */
        RTC_IntCmd(RTC_INT_PERIOD, ENABLE);
        /* Startup RTC count */
        RTC_Cmd(ENABLE);
    }
}

/**
 * @brief  XTAL32 clock initialize.
 * @param  None
 * @retval None
 */
static void XTAL32_ClkInit(void)
{
    stc_clock_xtal32_init_t stcXtal32Init;

    /* Xtal32 config */
    stcXtal32Init.u8State  = CLK_XTAL32_ON;
    stcXtal32Init.u8Drv    = CLK_XTAL32_DRV_HIGH;
    stcXtal32Init.u8Filter = CLK_XTAL32_FILTER_RUN_MD;
    (void)CLK_Xtal32Init(&stcXtal32Init);
    /* Waiting for XTAL32 stabilization */
    DDL_DelayMS(1000U);
}

/**
 * @brief  Main function of RTC calibration output.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure clock */
    BSP_CLK_Init();
    XTAL32_ClkInit();
    BSP_LED_Init();
    BSP_KEY_Init();

    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configure RTC */
    RTC_Config();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        /* K1 */
        if (SET == BSP_KEY_GetStatus(BSP_KEY_1)) {
            while (SET == BSP_KEY_GetStatus(BSP_KEY_1)) {
            }
            if (u16CompenVal < 255) {
                u16CompenVal++;
            }
            RTC_SetClockCompenValue((uint16_t)u16CompenVal & 0x1FFU);
        }
        /* K2 */
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            while (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            }
            if (u16CompenVal > -256) {
                u16CompenVal--;
            }
            RTC_SetClockCompenValue((uint16_t)u16CompenVal & 0x1FFU);
        }

        if (1U == u8SecIntFlag) {
            u8SecIntFlag = 0U;
            BSP_LED_Toggle(LED_RED);
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
