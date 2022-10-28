/**
 *******************************************************************************
 * @file  efm/efm_sequence_program/source/main.c
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
 * @addtogroup EFM_Sequence_Program
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EFM_SECTOR61_NUM                (61U)
#define EFM_SECTOR62_NUM                (62U)

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
    uint32_t u32Addr0, u32Addr1;
    uint8_t u8TestBuf[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    uint8_t u8ExpectBuf[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t u8ReadBuf1[20] = {0};
    uint8_t u8ReadBuf2[20] = {0};
    int32_t i32Ret1, i32Ret2;

    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();

    /* Enable flash. */
    EFM_Cmd(EFM_CHIP_ALL, ENABLE);

    while (SET != EFM_GetStatus(EFM_FLAG_RDY)) {
        ;
    }

    /* EFM_FWMC wirte enable */
    EFM_FWMC_Cmd(ENABLE);
    /* Release bus while erase & program */
    EFM_SetBusStatus(EFM_BUS_RELEASE);

    u32Addr0 = EFM_SECTOR_ADDR(EFM_SECTOR61_NUM);
    u32Addr1 = EFM_SECTOR_ADDR(EFM_SECTOR62_NUM);

    /* Erase sector 61 62. */
    (void)EFM_SectorErase(u32Addr0);
    (void)EFM_SectorErase(u32Addr1);

    /* Sequence program. */
    (void)EFM_SequenceProgram(u32Addr0, u8TestBuf, sizeof(u8TestBuf));
    (void)EFM_Program(u32Addr1, u8TestBuf, sizeof(u8TestBuf));

    (void)EFM_ReadByte(u32Addr0, u8ReadBuf1, sizeof(u8TestBuf));
    (void)EFM_ReadByte(u32Addr1, u8ReadBuf2, sizeof(u8TestBuf));

    i32Ret1 = memcmp(u8ReadBuf1, u8TestBuf, sizeof(u8TestBuf));
    i32Ret2 = memcmp(u8ReadBuf2, u8TestBuf, sizeof(u8TestBuf));
    if ((0 == i32Ret1) && (0 == i32Ret2)) {
        /* LED blue, as expected */
        BSP_LED_On(LED_BLUE);
    } else {
        /* LED red */
        BSP_LED_On(LED_RED);
    }

    EFM_ChipErase(EFM_CHIP_ALL);
    (void)EFM_ReadByte(u32Addr0, u8ReadBuf1, sizeof(u8ExpectBuf));
    (void)EFM_ReadByte(u32Addr1, u8ReadBuf2, sizeof(u8ExpectBuf));

    i32Ret1 = memcmp(u8ReadBuf1, u8ExpectBuf, sizeof(u8ExpectBuf));
    i32Ret2 = memcmp(u8ReadBuf2, u8ExpectBuf, sizeof(u8ExpectBuf));
    if ((0 == i32Ret1) && (0 == i32Ret2)) {
        /* LED blue, as expected */
        BSP_LED_On(LED_BLUE);
    } else {
        /* LED red */
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
