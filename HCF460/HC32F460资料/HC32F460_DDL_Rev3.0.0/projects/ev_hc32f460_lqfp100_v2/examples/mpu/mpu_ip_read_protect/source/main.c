/**
 *******************************************************************************
 * @file  mpu/mpu_ip_read_protect/source/main.c
 * @brief Main program of MPU IP write/read protect for the Device Driver Library.
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
 * @addtogroup MPU_IP_Read_Protect
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
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM | LL_PERIPH_MPU)
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

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Bus Fault callback
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    BSP_LED_On(LED_RED);
}

/**
 * @brief  KEY10 External interrupt handler function
 * @param  None
 * @retval None
 */
void BSP_KEY_KEY10_IrqHandler(void)
{
    uint32_t u32ThreadMode;

    if (SET == EXTINT_GetExtIntStatus(BSP_KEY_KEY10_EXTINT)) {
        u32ThreadMode = __get_CONTROL();
        if (CONTROL_nPRIV_Msk != (u32ThreadMode & CONTROL_nPRIV_Msk)) {
            /* User level */
            SET_REG32_BIT(u32ThreadMode, CONTROL_nPRIV_Msk);
            BSP_LED_Off(LED_BLUE);
        } else {
            /* Privilege level */
            CLR_REG32_BIT(u32ThreadMode, CONTROL_nPRIV_Msk);
            BSP_LED_On(LED_BLUE);
        }
        __set_CONTROL(u32ThreadMode);
        EXTINT_ClearExtIntStatus(BSP_KEY_KEY10_EXTINT);
    }
}

/**
 * @brief  MPU configuration.
 * @param  None
 * @retval None
 */
static void MPU_Config(void)
{
    /* Configure write/read protection of the IP */
    MPU_IP_SetExceptionType(MPU_IP_EXP_TYPE_BUS_ERR);
    MPU_IP_WriteCmd(MPU_IP_RTC, DISABLE);
    MPU_IP_ReadCmd(MPU_IP_RTC, DISABLE);
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
    stcRtcTime.u8AmPm   = RTC_HOUR_24H;
    if (LL_OK != RTC_SetDate(RTC_DATA_FMT_DEC, &stcRtcDate)) {
        DDL_Printf("Set Date failed!\r\n");
    }
    if (LL_OK != RTC_SetTime(RTC_DATA_FMT_DEC, &stcRtcTime)) {
        DDL_Printf("Set Time failed!\r\n");
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

    /* RTC stopped */
    if (DISABLE == RTC_GetCounterState()) {
        /* Reset RTC counter */
        if (LL_OK != RTC_DeInit()) {
            DDL_Printf("Reset RTC failed!\r\n");
        } else {
            /* Configure structure initialization */
            (void)RTC_StructInit(&stcRtcInit);
            /* Configuration RTC structure */
            stcRtcInit.u8ClockSrc   = RTC_CLK_SRC_LRC;
            stcRtcInit.u8HourFormat = RTC_HOUR_FMT_24H;
            stcRtcInit.u8IntPeriod  = RTC_INT_PERIOD_INVD;
            (void)RTC_Init(&stcRtcInit);
            /* Update date and time */
            RTC_CalendarConfig();
            /* Startup RTC count */
            RTC_Cmd(ENABLE);
        }
    }
}

/**
 * @brief  Main function of MPU IP write/read protect.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_rtc_date_t stcCurrentDate;
    stc_rtc_time_t stcCurrentTime;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configure RTC */
    RTC_Config();
    /* Configure MPU */
    MPU_Config();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    /* Get current thread mode */
    if (CONTROL_nPRIV_Msk != (__get_CONTROL() & CONTROL_nPRIV_Msk)) {
        /* Privilege level */
        BSP_LED_On(LED_BLUE);
    }
    /* Enable bus fault handler */
    SET_REG32_BIT(SCB->SHCSR, SCB_SHCSR_BUSFAULTENA_Msk);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            /* Get current date */
            if (LL_OK == RTC_GetDate(RTC_DATA_FMT_DEC, &stcCurrentDate)) {
                /* Get current time */
                if (LL_OK == RTC_GetTime(RTC_DATA_FMT_DEC, &stcCurrentTime)) {
                    /* Print current date and time */
                    DDL_Printf("20%02d/%02d/%02d %02d:%02d:%02d \r\n", stcCurrentDate.u8Year, stcCurrentDate.u8Month,
                               stcCurrentDate.u8Day, stcCurrentTime.u8Hour, stcCurrentTime.u8Minute, stcCurrentTime.u8Second);
                    BSP_LED_Off(LED_RED);
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
