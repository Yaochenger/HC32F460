/**
 *******************************************************************************
 * @file  rtc/rtc_low_power/source/main.c
 * @brief Main program of RTC Low Power for the Device Driver Library.
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
 * @addtogroup RTC_Low_Power
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
 * @brief  RTC calendar configuration.
 * @param  None
 * @retval None
 */
static void RTC_CalendarConfig(void)
{
    stc_rtc_date_t stcRtcDate;
    stc_rtc_time_t stcRtcTime;

    /* Date configuration */
    stcRtcDate.u8Year    = 20U;
    stcRtcDate.u8Month   = RTC_MONTH_JANUARY;
    stcRtcDate.u8Day     = 1U;
    stcRtcDate.u8Weekday = RTC_WEEKDAY_WEDNESDAY;

    /* Time configuration */
    stcRtcTime.u8Hour   = 23U;
    stcRtcTime.u8Minute = 59U;
    stcRtcTime.u8Second = 55U;
    stcRtcTime.u8AmPm   = RTC_HOUR_12H_AM;

    if (LL_OK != RTC_SetDate(RTC_DATA_FMT_DEC, &stcRtcDate)) {
        DDL_Printf("Set Date failed!\r\n");
    }

    if (LL_OK != RTC_SetTime(RTC_DATA_FMT_DEC, &stcRtcTime)) {
        DDL_Printf("Set Time failed!\r\n");
    }
}

/**
 * @brief  RTC display weekday.
 * @param  [in] u8Weekday               Weekday
 *         This parameter can be one of the following values:
 *           @arg RTC_WEEKDAY_SUNDAY:     Sunday
 *           @arg RTC_WEEKDAY_MONDAY:     Monday
 *           @arg RTC_WEEKDAY_TUESDAY:    Tuesday
 *           @arg RTC_WEEKDAY_WEDNESDAY:  Wednesday
 *           @arg RTC_WEEKDAY_THURSDAY:   Thursday
 *           @arg RTC_WEEKDAY_FRIDAY:     Friday
 *           @arg RTC_WEEKDAY_SATURDAY:   Saturday
 * @retval None
 */
static void RTC_DisplayWeekday(uint8_t u8Weekday)
{
    switch (u8Weekday) {
        case RTC_WEEKDAY_SUNDAY:
            DDL_Printf("Sunday\r\n");
            break;
        case RTC_WEEKDAY_MONDAY:
            DDL_Printf("Monday\r\n");
            break;
        case RTC_WEEKDAY_TUESDAY:
            DDL_Printf("Tuesday\r\n");
            break;
        case RTC_WEEKDAY_WEDNESDAY:
            DDL_Printf("Wednesday\r\n");
            break;
        case RTC_WEEKDAY_THURSDAY:
            DDL_Printf("Thursday\r\n");
            break;
        case RTC_WEEKDAY_FRIDAY:
            DDL_Printf("Friday\r\n");
            break;
        case RTC_WEEKDAY_SATURDAY:
            DDL_Printf("Saturday\r\n");
            break;
        default:
            break;
    }
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

    /* Reset RTC counter */
    if (LL_ERR_TIMEOUT == RTC_DeInit()) {
        DDL_Printf("Reset RTC failed!\r\n");
    } else {
        /* Configure structure initialization */
        (void)RTC_StructInit(&stcRtcInit);

        /* Configuration RTC structure */
        stcRtcInit.u8ClockSrc   = RTC_CLK_SRC_LRC;
        stcRtcInit.u8HourFormat = RTC_HOUR_FMT_24H;
        stcRtcInit.u8IntPeriod  = RTC_INT_PERIOD_PER_MINUTE;
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

        /* Update date and time */
        RTC_CalendarConfig();
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
 * @brief  Main function of RTC Low Power.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_rtc_date_t stcCurrentDate;
    stc_rtc_time_t stcCurrentTime;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure clock */
    BSP_CLK_Init();
    XTAL32_ClkInit();
    BSP_LED_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configure RTC */
    RTC_Config();

    /* Wait for RTC to work properly before switching to low power */
    if (RTC_ConfirmLPMCond() != LL_OK) {
        DDL_Printf("Switch to low power failed!\r\n");
    }
    PWC_SLEEP_Enter();
    /* Configure one seconds trigger interrupt */
    RTC_SetIntPeriod(RTC_INT_PERIOD_PER_SEC);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (1U == u8SecIntFlag) {
            u8SecIntFlag = 0U;
            BSP_LED_Toggle(LED_RED);
            /* Get current date */
            if (LL_OK == RTC_GetDate(RTC_DATA_FMT_BCD, &stcCurrentDate)) {
                /* Get current time */
                if (LL_OK == RTC_GetTime(RTC_DATA_FMT_BCD, &stcCurrentTime)) {
                    /* Print current date and time */
                    DDL_Printf("20%02x/%02x/%02x %02x:%02x:%02x ", stcCurrentDate.u8Year, stcCurrentDate.u8Month,
                               stcCurrentDate.u8Day, stcCurrentTime.u8Hour,
                               stcCurrentTime.u8Minute, stcCurrentTime.u8Second);
                    RTC_DisplayWeekday(stcCurrentDate.u8Weekday);
                } else {
                    DDL_Printf("Get time failed!\r\n");
                }
            } else {
                DDL_Printf("Get date failed!\r\n");
            }
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
