/**
 *******************************************************************************
 * @file  rmu/rmu_base/source/main.c
 * @brief Main program of RMU for the Device Driver Library.
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
 * @addtogroup RMU_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void PrintResetCause(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Print reset information.
 * @param  None
 * @retval None
 */
static void PrintResetCause(void)
{
    if (SET == RMU_GetStatus(RMU_FLAG_SW)) {
        DDL_Printf("Software reset.\r\n");
    }
    if (SET == RMU_GetStatus(RMU_FLAG_PIN)) {
        DDL_Printf("Pin reset.\r\n");
    }
    if (SET == RMU_GetStatus(RMU_FLAG_PWR_ON)) {
        DDL_Printf("Power on reset.\r\n");
    }
}

/**
 * @brief  Main function of RMU project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
    /* BSP clock initialize */
    BSP_CLK_Init();
    /* configuration uart for debug information */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* KEY initialize*/
    BSP_KEY_Init();
    /* Printf reset cause */
    PrintResetCause();
    /* Clear reset cause */
    RMU_ClearStatus();

    DDL_Printf("Press KEY10 to make software reset...\r\n");
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_10)) {
            while (RESET != BSP_KEY_GetStatus(BSP_KEY_10));
            /* Software reset MCU */
            NVIC_SystemReset();
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
