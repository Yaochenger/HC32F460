/**
 *******************************************************************************
 * @file  intc/intc_swint/source/main.c
 * @brief Main program SWINT for the Device Driver Library.
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
 * @addtogroup SWINT
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Software interrupt Ch.31 */
#define SWINT_CH            (SWINT_CH31)
#define SWINT_PRIO          (DDL_IRQ_PRIO_DEFAULT)

#define DLY_MS              (500UL)

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
 * @brief  Software interrupt Ch.31 callback function
 *         IRQ No.31 in Global is used for it
 * @param  None
 * @retval None
 */
static void SWINT31_Callback(void)
{
    BSP_LED_Toggle(LED_ALL);
    INTC_SWIntCmd(SWINT_CH31, DISABLE);
}

/**
 * @brief  Main function of SWINT project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
    /* BSP Clock initialize */
    BSP_CLK_Init();
    /* BSP LED initialize */
    BSP_LED_Init();
    /* SWINT Ch.31 configuration */
    INTC_SWIntInit(SWINT_CH, SWINT31_Callback, SWINT_PRIO);
    INTC_SWIntCmd(SWINT_CH, ENABLE);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
    for (;;) {
        INTC_SWIntCmd(SWINT_CH, ENABLE);
        DDL_DelayMS(DLY_MS);
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
