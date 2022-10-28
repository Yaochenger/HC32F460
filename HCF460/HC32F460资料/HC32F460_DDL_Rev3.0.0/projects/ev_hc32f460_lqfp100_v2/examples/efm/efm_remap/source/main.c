/**
 *******************************************************************************
 * @file  efm/efm_remap/source/main.c
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
 * @addtogroup EFM_Remap
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define ROM_REMAP_SIZE  EFM_REMAP_32K
#define ROM_REMAP_ADDR  (0x00000000UL)
#define ROM_REMAP_IDX   EFM_REMAP_IDX0

#define RAM_REMAP_SIZE  EFM_REMAP_8K
#define RAM_REMAP_ADDR  (0x1FFF8000UL + (1UL << RAM_REMAP_SIZE))
#define RAM_REMAP_IDX   EFM_REMAP_IDX1

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
    stc_efm_remap_init_t stcEfmRemapInit;
    uint32_t i32Ret;
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();

    EFM_REMAP_Unlock();
    /* ROM Remap */
    stcEfmRemapInit.u32State = EFM_REMAP_ON;
    stcEfmRemapInit.u32Addr = ROM_REMAP_ADDR;
    stcEfmRemapInit.u32Size = ROM_REMAP_SIZE;
    EFM_REMAP_Init(ROM_REMAP_IDX, &stcEfmRemapInit);

    /* RAM Remap (SRAMH only) */
    EFM_REMAP_SetSize(RAM_REMAP_IDX, RAM_REMAP_SIZE);
    EFM_REMAP_SetAddr(RAM_REMAP_IDX, RAM_REMAP_ADDR);
    EFM_REMAP_Cmd(RAM_REMAP_IDX, ENABLE);

    EFM_REMAP_Lock();

    i32Ret = memcmp((uint32_t *)stcEfmRemapInit.u32Addr, (uint32_t *)EFM_REMAP_BASE_ADDR0, (1UL << ROM_REMAP_SIZE));
    i32Ret |= memcmp((uint32_t *)RAM_REMAP_ADDR, (uint32_t *)EFM_REMAP_BASE_ADDR1, (1UL << RAM_REMAP_SIZE));

    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    if (0 == i32Ret) {
        /* LED blue, as expected */
        BSP_LED_On(LED_BLUE);
    } else {
        /* LED red */
        BSP_LED_On(LED_RED);
    }

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
