/**
 *******************************************************************************
 * @file  crc/crc_hw_encode_hw_check/source/main.c
 * @brief This example demonstrates CRC base function.
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
 * @addtogroup CRC_HW_Encode_HW_Check
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU)

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
 * @brief  Main function of CRC base project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint16_t u16CrcValue;
    uint32_t u32CrcValue;
    uint32_t u32CrcErrorCount = 0UL;
    en_flag_status_t enCrcMatch;
    stc_crc_init_t stcCrcInit;
    const uint8_t  au8SrcData[5U]  = {0x12U, 0x34, 0x56, 0x78, 0x90};
    const uint16_t au16SrcData[5U] = {0x1234U, 0x3456U, 0x7890U, 0x90AB, 0xABCD};
    const uint32_t au32SrcData[5U] = {0x12345678UL, 0x34567890UL, 0x567890ABUL, 0x7890ABCDUL, 0x90ABCDEFUL};

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable CRC module clock. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_CRC, ENABLE);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /******** Calculates CRC16 value by writing data in byte. **************/
    stcCrcInit.u32Protocol = CRC_CRC16;
    stcCrcInit.u32InitValue = CRC16_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_DISABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_DISABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_DISABLE;
    (void)CRC_Init(&stcCrcInit);

    u16CrcValue = (uint16_t)CRC_AccumulateData8(au8SrcData, ARRAY_SZ(au8SrcData));
    enCrcMatch = CRC_CheckData8(CRC16_INIT_VALUE, au8SrcData, ARRAY_SZ(au8SrcData), u16CrcValue);
    if (enCrcMatch != SET) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC16 value by writing data in half-word. **************/
    stcCrcInit.u32Protocol = CRC_CRC16;
    stcCrcInit.u32InitValue = CRC16_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_ENABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_DISABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_DISABLE;
    (void)CRC_Init(&stcCrcInit);

    u16CrcValue = (uint16_t)CRC_AccumulateData16(au16SrcData, ARRAY_SZ(au16SrcData));
    enCrcMatch = CRC_CheckData16(CRC16_INIT_VALUE, au16SrcData, ARRAY_SZ(au16SrcData), u16CrcValue);
    if (enCrcMatch != SET) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC16 value by writing data in word. **************/
    stcCrcInit.u32Protocol = CRC_CRC16;
    stcCrcInit.u32InitValue = CRC16_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_DISABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_ENABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_DISABLE;
    (void)CRC_Init(&stcCrcInit);

    u16CrcValue = (uint16_t)CRC_AccumulateData32(au32SrcData, ARRAY_SZ(au32SrcData));
    enCrcMatch = CRC_CheckData32(CRC16_INIT_VALUE, au32SrcData, ARRAY_SZ(au32SrcData), u16CrcValue);
    if (enCrcMatch != SET) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC32 value by writing data in byte. **************/
    stcCrcInit.u32Protocol = CRC_CRC32;
    stcCrcInit.u32InitValue = CRC32_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_DISABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_DISABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_ENABLE;
    (void)CRC_Init(&stcCrcInit);

    u32CrcValue = CRC_AccumulateData8(au8SrcData, ARRAY_SZ(au8SrcData));
    enCrcMatch = CRC_CheckData8(CRC32_INIT_VALUE, au8SrcData, ARRAY_SZ(au8SrcData), u32CrcValue);
    if (enCrcMatch != SET) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC32 value by writing data in half-word. *********/
    stcCrcInit.u32Protocol = CRC_CRC32;
    stcCrcInit.u32InitValue = CRC32_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_ENABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_ENABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_DISABLE;
    (void)CRC_Init(&stcCrcInit);

    u32CrcValue = CRC_AccumulateData16(au16SrcData, ARRAY_SZ(au16SrcData));
    enCrcMatch = CRC_CheckData16(CRC32_INIT_VALUE, au16SrcData, ARRAY_SZ(au16SrcData), u32CrcValue);
    if (enCrcMatch != SET) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC32 value by writing data in word. **************/
    stcCrcInit.u32Protocol = CRC_CRC32;
    stcCrcInit.u32InitValue = CRC32_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_ENABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_ENABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_ENABLE;
    (void)CRC_Init(&stcCrcInit);

    u32CrcValue = CRC_AccumulateData32(au32SrcData, ARRAY_SZ(au32SrcData));
    enCrcMatch = CRC_CheckData32(CRC32_INIT_VALUE, au32SrcData, ARRAY_SZ(au32SrcData), u32CrcValue);
    if (enCrcMatch != SET) {
        u32CrcErrorCount++;
    }

    /* Check test result */
    if (0UL == u32CrcErrorCount) {
        BSP_LED_On(LED_BLUE);   /* Test result meets the expected. */
    } else {
        BSP_LED_On(LED_RED);    /* Test result doesn't meet the expected. */
    }

    for (;;) {
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
