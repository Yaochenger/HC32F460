/**
 *******************************************************************************
 * @file  efm/efm_base/source/main.c
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
 * @addtogroup EFM_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EFM_SECTOR10_NUM        (10U)

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
    uint32_t u32Data = 0xAA5555AAU;
    uint32_t u32Addr;
    uint32_t u32ReadData1, u32ReadData2;
    int32_t i32Ret1, i32Ret2;

    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();

    /* Wait flash ready. */
    while (SET != EFM_GetStatus(EFM_FLAG_RDY)) {
        ;
    }

    /* EFM_FWMC wirte enable */
    EFM_FWMC_Cmd(ENABLE);

    /* Erase sector 10. sector 10: 0x00014000~0x00015FFF */
    u32Addr = EFM_SECTOR_ADDR(EFM_SECTOR10_NUM);
    (void)EFM_SectorErase(u32Addr);

    /* Single program */
    if (LL_OK != EFM_ProgramWord(u32Addr, u32Data)) {
        BSP_LED_On(LED_RED);
    }
    (void)EFM_ReadByte(u32Addr, (uint8_t *)&u32ReadData1, sizeof(u32Data));
    i32Ret1 = memcmp(&u32ReadData1, &u32Data, sizeof(u32Data));

    u32Addr += sizeof(u32Data);
    /* Single program read back */
    if (LL_OK != EFM_ProgramWordReadBack(u32Addr, u32Data)) {
        BSP_LED_On(LED_RED);
    }
    (void)EFM_ReadByte(u32Addr, (uint8_t *)&u32ReadData2, sizeof(u32Data));
    i32Ret2 = memcmp(&u32ReadData2, &u32Data, sizeof(u32Data));

    EFM_FWMC_Cmd(DISABLE);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    if ((0 == i32Ret1) && (0 == i32Ret2)) {
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
