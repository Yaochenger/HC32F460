/**
 *******************************************************************************
 * @file  wdt/wdt_interrupt_sw_startup/source/main.c
 * @brief Main program of WDT Interrupt for the Device Driver Library.
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
 * @addtogroup WDT_Interrupt
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

/* WDT definition */
#define WDT_INT_SRC                     (INT_SRC_WDT_REFUDF)
#define WDT_IRQ_NUM                     (INT006_IRQn)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8ExtIntCount = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  WDT interrupt callback function.
 * @param  None
 * @retval None
 */
static void WDT_IrqCallback(void)
{
    en_flag_status_t enFlagSta;

    enFlagSta = WDT_GetStatus(WDT_FLAG_UDF);
    /* WDT underflow interrupt */
    if (SET == enFlagSta) {
        (void)WDT_ClearStatus(WDT_FLAG_UDF);
        /* Normal mode */
        if (0U == u8ExtIntCount) {
            BSP_LED_Toggle(LED_RED);
        } else {
            /* Sleep mode */
            BSP_LED_Toggle(LED_BLUE);
        }
    }
    WDT_FeedDog();
}

/**
 * @brief  KEY10 External interrupt handler function
 * @param  None
 * @retval None
 */
void BSP_KEY_KEY10_IrqHandler(void)
{
    if (SET == EXTINT_GetExtIntStatus(BSP_KEY_KEY10_EXTINT)) {
        u8ExtIntCount++;
        if (u8ExtIntCount >= 2U) {
            u8ExtIntCount = 0U;
        }
        BSP_LED_Off(LED_RED);
        BSP_LED_Off(LED_BLUE);
        EXTINT_ClearExtIntStatus(BSP_KEY_KEY10_EXTINT);
    }
}

/**
 * @brief  WDT configuration.
 * @param  None
 * @retval None
 */
static void WDT_Config(void)
{
    stc_wdt_init_t stcWdtInit;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* WDT configuration */
    stcWdtInit.u32CountPeriod   = WDT_CNT_PERIOD16384;
    stcWdtInit.u32ClockDiv      = WDT_CLK_DIV512;
    stcWdtInit.u32RefreshRange  = WDT_RANGE_0TO100PCT;
    stcWdtInit.u32LPMCount      = WDT_LPM_CNT_CONTINUE;
    stcWdtInit.u32ExceptionType = WDT_EXP_TYPE_INT;
    (void)WDT_Init(&stcWdtInit);

    /* Interrupt configuration */
    stcIrqSignConfig.enIntSrc    = WDT_INT_SRC;
    stcIrqSignConfig.enIRQn      = WDT_IRQ_NUM;
    stcIrqSignConfig.pfnCallback = &WDT_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
}

/**
 * @brief  Main function of WDT Interrupt.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Configure WDT */
    WDT_Config();
    /* First reload counter to start WDT */
    WDT_FeedDog();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        /* Sleep mode */
        if (1U == u8ExtIntCount) {
            PWC_SLEEP_Enter();
        } else {
            /* Reserved */
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
