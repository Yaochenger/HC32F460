/**
 *******************************************************************************
 * @file  efm/efm_swap/source/main.c
 * @brief Main program of EFM for the Device Driver Library.
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
 * @addtogroup EFM_Swap
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

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of EFM project
 * @param  None
 * @retval int32_t return value, if needed
 */

int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* LED init */
    BSP_LED_Init();
    /* KEY Init */
    BSP_KEY_Init();

    /* Determine whether the boot swap is enabled */
    if (RW_MEM32(EFM_SWAP_ADDR) == EFM_SWAP_DATA) {
        BSP_LED_On(LED_RED); /* boot swap is on */
    } else {
        BSP_LED_On(LED_BLUE); /* boot swap is off */
    }

    /* EFM_FWMC wirte enable */
    EFM_FWMC_Cmd(ENABLE);

    /* Wait flash ready. */
    while (SET != EFM_GetStatus(EFM_FLAG_RDY)) {
        ;
    }

    /* Register write protected for some required peripherals. */
    //LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_SRAM);

    for (;;) {
        /* KEY1 */
        if (SET == BSP_KEY_GetStatus(BSP_KEY_1)) {
            (void)EFM_SwapCmd(ENABLE);
        }
        /* KEY2 */
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            (void)EFM_SwapCmd(DISABLE);
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
