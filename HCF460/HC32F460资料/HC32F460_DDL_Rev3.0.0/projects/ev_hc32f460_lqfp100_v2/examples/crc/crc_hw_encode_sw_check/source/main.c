/**
 *******************************************************************************
 * @file  crc/crc_hw_encode_sw_check/source/main.c
 * @brief This example demonstrates CRC compare with software.
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
 * @addtogroup CRC_HW_Encode_SW_Check
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

#define SW_CRC_DATA_WIDTH_8BIT          (1U)
#define SW_CRC_DATA_WIDTH_16BIT         (2U)
#define SW_CRC_DATA_WIDTH_32BIT         (4U)

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
 * @brief CRC-16 calculation.
 * @param [in] u16InitValue          Initialize the CRC calculation.
 * @param [in] u8DataWidth           Width of the data.
 * @param [in] pu8Data               Pointer to the buffer containing the data to be computed.
 * @param [in] u32Len                The length of the data to be computed.
 * @retval CRC-16 calculation result
 */
static uint16_t CRC16_SW_Calculate(uint16_t u16InitValue, uint8_t u8DataWidth, const uint8_t *pu8Data, uint32_t u32Len)
{
    uint8_t i;
    uint8_t j;
    uint32_t u32Temp = (uint32_t)pu8Data;
    uint16_t u16Crc = u16InitValue;     /*  Initial value */

    while (u32Len > 0UL) {
        i = u8DataWidth;
        while (i > 0U) {
            u16Crc ^= *(uint8_t *)u32Temp;
            u32Temp++;
            for (j = 0U; j < 8U; j++) {
                if ((u16Crc & 0x1UL) != 0UL) {
                    u16Crc >>= 1U;
                    u16Crc ^= 0x8408U;  /* 0x8408 = reverse 0x1021 */
                } else {
                    u16Crc >>= 1U;
                }
            }
            i--;
        }
        u32Len--;
    }

    u16Crc = ~u16Crc;

    return u16Crc;
}

/**
 * @brief CRC-32 calculation.
 * @param [in] u32InitValue          Initialize the CRC calculation.
 * @param [in] u8DataWidth           Width of the data.
 * @param [in] pu8Data               Pointer to the buffer containing the data to be computed.
 * @param [in] u32Len                The length of the data to be computed.
 * @retval CRC-32 calculation result
 */
static uint32_t CRC32_SW_Calculate(uint32_t u32InitValue, uint8_t u8DataWidth, const uint8_t *pu8Data, uint32_t u32Len)
{
    uint8_t i;
    uint8_t j;
    uint32_t u32Temp = (uint32_t)pu8Data;
    uint32_t u32Crc = u32InitValue; /* Initial value */

    while (u32Len > 0UL) {
        i = u8DataWidth;
        while (i > 0U) {
            u32Crc ^= *(uint8_t *)u32Temp;
            u32Temp++;
            for (j = 0U; j < 8U; j++) {
                if ((u32Crc & 0x1UL) != 0UL) {
                    u32Crc = (u32Crc >> 1U) ^ 0xEDB88320UL; /*0xEDB88320UL = reverse 0x04C11DB7UL*/
                } else {
                    u32Crc = (u32Crc >> 1U);
                }
            }
            i--;
        }
        u32Len--;
    }

    u32Crc = ~u32Crc;

    return u32Crc;
}

/**
 * @brief  Main function of CRC software project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    const uint8_t  au8SrcData [3U] = {0x12U, 0x21U, 0U};
    const uint16_t au16SrcData[3U] = {0x1234U, 0x4321U, 0U};
    const uint32_t au32SrcData[3U] = {0x12345678UL, 0x87654321UL, 0UL};
    uint16_t u16SwCrcValue;
    uint16_t u16HwCrcValue;
    uint32_t u32SwCrcValue;
    uint32_t u32HwCrcValue;
    uint32_t u32CrcErrorCount = 0UL;
    stc_crc_init_t stcCrcInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable CRC module clock. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_CRC, ENABLE);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /******** Initialize CRC16. ***********************************************/
    stcCrcInit.u32Protocol = CRC_CRC16;
    stcCrcInit.u32InitValue = CRC16_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_ENABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_ENABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_ENABLE;
    (void)CRC_Init(&stcCrcInit);

    /******** Calculates CRC16 checksum by writing data in byte. **************/
    u16SwCrcValue = CRC16_SW_Calculate((uint16_t)CRC16_INIT_VALUE, SW_CRC_DATA_WIDTH_8BIT, au8SrcData, ARRAY_SZ(au8SrcData));
    u16HwCrcValue = (uint16_t)CRC_AccumulateData8(au8SrcData, ARRAY_SZ(au8SrcData));
    if (u16SwCrcValue != u16HwCrcValue) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC16 checksum by writing data in half-word. *********/
    u16SwCrcValue = CRC16_SW_Calculate((uint16_t)CRC16_INIT_VALUE, SW_CRC_DATA_WIDTH_16BIT, (const uint8_t *)au16SrcData, ARRAY_SZ(au16SrcData));
    u16HwCrcValue = (uint16_t)CRC_CalculateData16(CRC16_INIT_VALUE, au16SrcData, ARRAY_SZ(au16SrcData));
    if (u16SwCrcValue != u16HwCrcValue) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC16 checksum by writing data in word. **************/
    u16SwCrcValue = CRC16_SW_Calculate((uint16_t)CRC16_INIT_VALUE, SW_CRC_DATA_WIDTH_32BIT, (const uint8_t *)au32SrcData, ARRAY_SZ(au32SrcData));
    u16HwCrcValue = (uint16_t)CRC_CalculateData32(CRC16_INIT_VALUE, au32SrcData, ARRAY_SZ(au32SrcData));
    if (u16SwCrcValue != u16HwCrcValue) {
        u32CrcErrorCount++;
    }

    /******** Initialize CRC32. ***********************************************/
    stcCrcInit.u32Protocol = CRC_CRC32;
    stcCrcInit.u32InitValue = CRC32_INIT_VALUE;
    stcCrcInit.u32RefIn = CRC_REFIN_ENABLE;
    stcCrcInit.u32RefOut = CRC_REFOUT_ENABLE;
    stcCrcInit.u32XorOut = CRC_XOROUT_ENABLE;
    (void)CRC_Init(&stcCrcInit);

    /******** Calculates CRC32 checksum by writing data in byte. **************/
    u32SwCrcValue = CRC32_SW_Calculate(CRC32_INIT_VALUE, SW_CRC_DATA_WIDTH_8BIT, au8SrcData, ARRAY_SZ(au8SrcData));
    u32HwCrcValue = CRC_AccumulateData8(au8SrcData, ARRAY_SZ(au8SrcData));
    if (u32SwCrcValue != u32HwCrcValue) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC32 checksum by writing data in half-word. *********/
    u32SwCrcValue = CRC32_SW_Calculate(CRC32_INIT_VALUE, SW_CRC_DATA_WIDTH_16BIT, (const uint8_t *)au16SrcData, ARRAY_SZ(au16SrcData));
    u32HwCrcValue = CRC_CalculateData16(CRC32_INIT_VALUE, au16SrcData, ARRAY_SZ(au16SrcData));
    if (u32SwCrcValue != u32HwCrcValue) {
        u32CrcErrorCount++;
    }

    /******** Calculates CRC32 checksum by writing data in word. **************/
    u32SwCrcValue = CRC32_SW_Calculate(CRC32_INIT_VALUE, SW_CRC_DATA_WIDTH_32BIT, (const uint8_t *)au32SrcData, ARRAY_SZ(au32SrcData));
    u32HwCrcValue = CRC_CalculateData32(CRC32_INIT_VALUE, au32SrcData, ARRAY_SZ(au32SrcData));
    if (u32SwCrcValue != u32HwCrcValue) {
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
