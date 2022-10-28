/**
 *******************************************************************************
 * @file  efm/efm_otp/source/main.c
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
 * @addtogroup EFM_Otp
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EFM_OTP_BLOCK_NUM       (14U)

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
    uint32_t u32Data = 0x5A5A5A5AU;

    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();
    /* KEY Init */
    BSP_KEY_Init();

    /* Turn on LED blue */
    BSP_LED_On(LED_BLUE);

    /* Wait flash ready. */
    while (SET != EFM_GetStatus(EFM_FLAG_RDY)) {
        ;
    }

    /* EFM_FWMC wirte enable */
    EFM_FWMC_Cmd(ENABLE);
    /* Program to specified otp block */
    EFM_ProgramWord(EFM_OTP_BLOCK14, u32Data);

    /* K1 */
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_1)) {
        ;
    }

    /* Lock OTP block, please ensure lock block number */
    (void)EFM_OTP_Lock(EFM_OTP_BLOCK_LOCKADDR(EFM_OTP_BLOCK_NUM));
    /* Program to lock otp block */
    EFM_ProgramWord(EFM_OTP_BLOCK14 + 8UL, u32Data);

    if (u32Data != RW_MEM32(EFM_OTP_BLOCK14 + 8UL)) {
        BSP_LED_Off(LED_BLUE);
        BSP_LED_On(LED_RED);
    }

    EFM_FWMC_Cmd(DISABLE);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    for (;;) {
        ;
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
