/**
 *******************************************************************************
 * @file  rtc/rtc_alarm/source/main.c
 * @brief Main program of RTC Alarm for the Device Driver Library.
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
 * @addtogroup RTC_Alarm
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
static uint8_t u8AlarmIntFlag = 0U, u8AlarmCnt = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  RTC period interrupt callback function.
 * @param  None
 * @retval None
 */
void RTC_Period_IrqHandler(void)
{
    u8SecIntFlag = 1U;
}

/**
 * @brief  RTC alarm interrupt callback function.
 * @param  None
 * @retval None
 */
static void RTC_Alarm_IrqHandler(void)
{
    u8AlarmCnt = 10U;
    u8AlarmIntFlag = 1U;
    RTC_ClearStatus(RTC_FLAG_ALARM);
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
    stcRtcTime.u8Hour   = 11U;
    stcRtcTime.u8Minute = 59U;
    stcRtcTime.u8Second = 55U;
    stcRtcTime.u8AmPm   = RTC_HOUR_12H_PM;

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
    stc_rtc_init_t stcRtcInit;
    stc_rtc_alarm_t stcRtcAlarm;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Reset RTC counter */
    if (LL_ERR_TIMEOUT == RTC_DeInit()) {
        DDL_Printf("Reset RTC failed!\r\n");
    } else {
        /* Configure structure initialization */
        (void)RTC_StructInit(&stcRtcInit);

        /* Configuration RTC structure */
        stcRtcInit.u8ClockSrc   = RTC_CLK_SRC_XTAL32;
        stcRtcInit.u8HourFormat = RTC_HOUR_FMT_12H;
        stcRtcInit.u8IntPeriod  = RTC_INT_PERIOD_PER_SEC;
        (void)RTC_Init(&stcRtcInit);

        /* Configuration alarm clock time: Monday to Friday, PM 12:00 */
        stcRtcAlarm.u8AlarmHour    = 0x12U;
        stcRtcAlarm.u8AlarmMinute  = 0x00U;
        stcRtcAlarm.u8AlarmWeekday = RTC_ALARM_WEEKDAY_MONDAY    | RTC_ALARM_WEEKDAY_TUESDAY  |
                                     RTC_ALARM_WEEKDAY_WEDNESDAY | RTC_ALARM_WEEKDAY_THURSDAY |
                                     RTC_ALARM_WEEKDAY_FRIDAY;
        stcRtcAlarm.u8AlarmAmPm    = RTC_HOUR_12H_AM;
        (void)RTC_SetAlarm(RTC_DATA_FMT_BCD, &stcRtcAlarm);
        RTC_AlarmCmd(ENABLE);

        /* Update date and time */
        RTC_CalendarConfig();
        /* RTC period and alarm interrupt configure */
        stcIrqSignConfig.enIntSrc    = INT_SRC_RTC_ALM;
        stcIrqSignConfig.enIRQn      = INT001_IRQn;
        stcIrqSignConfig.pfnCallback = &RTC_Alarm_IrqHandler;
        (void)INTC_IrqSignIn(&stcIrqSignConfig);
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        stcIrqSignConfig.enIntSrc    = INT_SRC_RTC_PRD;
        stcIrqSignConfig.enIRQn      = INT002_IRQn;
        stcIrqSignConfig.pfnCallback = &RTC_Period_IrqHandler;
        (void)INTC_IrqSignIn(&stcIrqSignConfig);
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        /* Enable period and alarm interrupt */
        RTC_IntCmd((RTC_INT_PERIOD | RTC_INT_ALARM), ENABLE);
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
 * @brief  RTC display date and time.
 * @param  [in] u8Format                Specifies the format of the entered parameters.
 *         This parameter can be one of the following values:
 *           @arg RTC_DATA_FMT_DEC:  Decimal data format
 *           @arg RTC_DATA_FMT_BCD:  BCD data format
 * @param  [in] pcTitle                 Pointer to a title information of display
 * @param  [in] pstcRtcDate             Pointer to a @ref stc_rtc_date_t structure
 * @param  [in] pstcRtcTime             Pointer to a @ref stc_rtc_time_t structure
 * @retval None
 *
 */
static void RTC_DisplayDataTime(uint8_t u8Format, const char *pcTitle, const stc_rtc_date_t *pstcRtcDate,
                                const stc_rtc_time_t *pstcRtcTime)
{
    if (RTC_DATA_FMT_BCD == u8Format) {
        DDL_Printf("%s 20%02x/%02x/%02x %02x:%02x:%02x ", pcTitle, pstcRtcDate->u8Year, pstcRtcDate->u8Month,
                   pstcRtcDate->u8Day, pstcRtcTime->u8Hour,
                   pstcRtcTime->u8Minute, pstcRtcTime->u8Second);
    } else {
        DDL_Printf("%s 20%02d/%02d/%02d %02d:%02d:%02d ", pcTitle, pstcRtcDate->u8Year, pstcRtcDate->u8Month,
                   pstcRtcDate->u8Day, pstcRtcTime->u8Hour,
                   pstcRtcTime->u8Minute, pstcRtcTime->u8Second);
    }

    if (RTC_HOUR_12H_AM == pstcRtcTime->u8AmPm) {
        DDL_Printf("Am ");
    } else {
        DDL_Printf("Pm ");
    }
    RTC_DisplayWeekday(pstcRtcDate->u8Weekday);
}

/**
 * @brief  Main function of RTC Alarm.
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
    /* Configure BSP */
    BSP_LED_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configure RTC */
    RTC_Config();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (1U == u8SecIntFlag) {
            u8SecIntFlag = 0U;
            /* Print alarm information */
            if ((1U == u8AlarmIntFlag) && (u8AlarmCnt > 0U)) {
                /* Alarm LED flicker */
                BSP_LED_Toggle(LED_RED);
                u8AlarmCnt--;
                if (0U == u8AlarmCnt) {
                    u8AlarmIntFlag = 0U;
                    BSP_LED_Off(LED_RED);
                }
                /* Get alarm date */
                if (LL_OK == RTC_GetDate(RTC_DATA_FMT_BCD, &stcCurrentDate)) {
                    /* Get alarm time */
                    if (LL_OK == RTC_GetTime(RTC_DATA_FMT_BCD, &stcCurrentTime)) {
                        /* Print alarm date and time */
                        RTC_DisplayDataTime(RTC_DATA_FMT_BCD, "Alarm", &stcCurrentDate, &stcCurrentTime);
                    } else {
                        DDL_Printf("Get alarm time failed!\r\n");
                    }
                } else {
                    DDL_Printf("Get alarm date failed!\r\n");
                }
            } else { /* Print current date and time */
                /* Get current date */
                if (LL_OK == RTC_GetDate(RTC_DATA_FMT_DEC, &stcCurrentDate)) {
                    /* Get current time */
                    if (LL_OK == RTC_GetTime(RTC_DATA_FMT_DEC, &stcCurrentTime)) {
                        /* Print current date and time */
                        RTC_DisplayDataTime(RTC_DATA_FMT_DEC, "Normal", &stcCurrentDate, &stcCurrentTime);
                    } else {
                        DDL_Printf("Get time failed!\r\n");
                    }
                } else {
                    DDL_Printf("Get date failed!\r\n");
                }
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
