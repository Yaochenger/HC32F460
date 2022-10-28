/**
 *******************************************************************************
 * @file  qspi/qspi_base/source/qspi_flash.c
 * @brief This file provides firmware functions to the QSPI accesses flash.
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
#include "qspi_flash.h"

/**
 * @addtogroup HC32F460_DDL_Examples
 * @{
 */

/**
 * @addtogroup QSPI_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define QSPI_FLASH_ENTER_XIP_MD         (0x20U)
#define QSPI_FLASH_EXIT_XIP_MD          (0xFFU)

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
 * @defgroup QSPI_FLASH_Global_Functions QSPI_FLASH Global Functions
 * @{
 */
/**
 * @brief  Convert word to bytes.
 * @param  [in] u32Word                 The word value.
 * @param  [in] pu8Byte                 Pointer to the byte buffer.
 * @retval None
 */
static void QSPI_FLASH_WordToByte(uint32_t u32Word, uint8_t *pu8Byte)
{
    uint32_t u32ByteNum;
    uint8_t u8Count = 0U;

    u32ByteNum = QSPI_FLASH_ADDR_WIDTH;
    do {
        pu8Byte[u8Count++] = (uint8_t)(u32Word >> (u32ByteNum * 8U)) & 0xFFU;
    } while ((u32ByteNum--) != 0UL);
}

/**
 * @brief  QSPI write instruction.
 * @param  [in] u8Instr                 The instruction code.
 * @param  [in] pu8Addr                 Pointer to the address buffer.
 * @param  [in] u32AddrLen              Size of address buffer.
 * @param  [in] pu8WriteBuf             Pointer to the write buffer.
 * @param  [in] u32BufLen               Size of write buffer.
 * @retval None
 */
static void QSPI_FLASH_WriteInstr(uint8_t u8Instr, uint8_t *pu8Addr, uint32_t u32AddrLen,
                                  uint8_t *pu8WriteBuf, uint32_t u32BufLen)
{
    uint32_t u32Count;

    QSPI_EnterDirectCommMode();
    QSPI_WriteDirectCommValue(u8Instr);
    if ((NULL != pu8Addr) && (0UL != u32AddrLen)) {
        for (u32Count = 0UL; u32Count < u32AddrLen; u32Count++) {
            QSPI_WriteDirectCommValue(pu8Addr[u32Count]);
        }
    }
    if ((NULL != pu8WriteBuf) && (0UL != u32BufLen)) {
        for (u32Count = 0UL; u32Count < u32BufLen; u32Count++) {
            QSPI_WriteDirectCommValue(pu8WriteBuf[u32Count]);
        }
    }
    QSPI_ExitDirectCommMode();
}

/**
 * @brief  QSPI read instruction.
 * @param  [in] u8Instr                 The instruction code.
 * @param  [in] pu8Addr                 Pointer to the address buffer.
 * @param  [in] u32AddrLen              Size of address buffer.
 * @param  [out] pu8ReadBuf             Pointer to the read buffer.
 * @param  [in] u32BufLen               Size of read buffer.
 * @retval None
 */
static void QSPI_FLASH_ReadInstr(uint8_t u8Instr, uint8_t *pu8Addr, uint32_t u32AddrLen,
                                 uint8_t *pu8ReadBuf, uint32_t u32BufLen)
{
    uint32_t u32Count;

    QSPI_EnterDirectCommMode();
    QSPI_WriteDirectCommValue(u8Instr);
    if ((NULL != pu8Addr) && (0UL != u32AddrLen)) {
        for (u32Count = 0UL; u32Count < u32AddrLen; u32Count++) {
            QSPI_WriteDirectCommValue(pu8Addr[u32Count]);
        }
    }
    if ((NULL != pu8ReadBuf) && (0UL != u32BufLen)) {
        for (u32Count = 0UL; u32Count < u32BufLen; u32Count++) {
            pu8ReadBuf[u32Count] = QSPI_ReadDirectCommValue();
        }
    }
    QSPI_ExitDirectCommMode();
}

/**
 * @brief  QSPI check process done.
 * @param  u32Timeout                   The timeout times (ms).
 * @retval int32_t:
 *           - LL_OK: No errors occurred.
 *           - LL_ERR_TIMEOUT: Works timeout.
 */
static int32_t QSPI_FLASH_CheckProcessDone(uint32_t u32Timeout)
{
    uint8_t u8Status;
    uint32_t u32Count;
    int32_t i32Ret = LL_ERR_TIMEOUT;

    u32Count = u32Timeout * (HCLK_VALUE / 20000UL);
    QSPI_EnterDirectCommMode();
    QSPI_WriteDirectCommValue(W25Q64_RD_STATUS_REG1);
    while ((u32Count--) != 0UL) {
        u8Status = QSPI_ReadDirectCommValue();
        if (0U == (u8Status & W25Q64_FLAG_BUSY)) {
            i32Ret = LL_OK;
            break;
        }
    }
    QSPI_ExitDirectCommMode();

    return i32Ret;
}

/**
 * @brief  De-initializes QSPI.
 * @param  None
 * @retval None
 */
void QSPI_FLASH_DeInit(void)
{
    (void)QSPI_DeInit();
}

/**
 * @brief  Initialize the QSPI Flash.
 * @param  None
 * @retval int32_t:
 *           - LL_OK: Initialize success
 *           - LL_ERR_INVD_PARAM: Invalid parameter
 */
void QSPI_FLASH_Init(void)
{
    stc_qspi_init_t stcQspiInit;
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    (void)GPIO_Init(QSPI_FLASH_CS_PORT,  QSPI_FLASH_CS_PIN,  &stcGpioInit);
    (void)GPIO_Init(QSPI_FLASH_SCK_PORT, QSPI_FLASH_SCK_PIN, &stcGpioInit);
    (void)GPIO_Init(QSPI_FLASH_IO0_PORT, QSPI_FLASH_IO0_PIN, &stcGpioInit);
    (void)GPIO_Init(QSPI_FLASH_IO1_PORT, QSPI_FLASH_IO1_PIN, &stcGpioInit);
    (void)GPIO_Init(QSPI_FLASH_IO2_PORT, QSPI_FLASH_IO2_PIN, &stcGpioInit);
    (void)GPIO_Init(QSPI_FLASH_IO3_PORT, QSPI_FLASH_IO3_PIN, &stcGpioInit);
    GPIO_SetFunc(QSPI_FLASH_CS_PORT,  QSPI_FLASH_CS_PIN,  QSPI_FLASH_CS_FUNC);
    GPIO_SetFunc(QSPI_FLASH_SCK_PORT, QSPI_FLASH_SCK_PIN, QSPI_FLASH_SCK_FUNC);
    GPIO_SetFunc(QSPI_FLASH_IO0_PORT, QSPI_FLASH_IO0_PIN, QSPI_FLASH_IO0_FUNC);
    GPIO_SetFunc(QSPI_FLASH_IO1_PORT, QSPI_FLASH_IO1_PIN, QSPI_FLASH_IO1_FUNC);
    GPIO_SetFunc(QSPI_FLASH_IO2_PORT, QSPI_FLASH_IO2_PIN, QSPI_FLASH_IO2_FUNC);
    GPIO_SetFunc(QSPI_FLASH_IO3_PORT, QSPI_FLASH_IO3_PIN, QSPI_FLASH_IO3_FUNC);

    FCG_Fcg1PeriphClockCmd(QSPI_FLASH_CLK, ENABLE);
    (void)QSPI_StructInit(&stcQspiInit);
    stcQspiInit.u32ClockDiv       = QSPI_CLK_DIV3;
    stcQspiInit.u32ReadMode       = QSPI_FLASH_RD_MD;
    stcQspiInit.u32PrefetchMode   = QSPI_PREFETCH_MD_EDGE_STOP;
    stcQspiInit.u32DummyCycle     = QSPI_FLASH_RD_DUMMY_CYCLE;
    stcQspiInit.u32AddrWidth      = QSPI_FLASH_ADDR_WIDTH;
    stcQspiInit.u32SetupTime      = QSPI_QSSN_SETUP_ADVANCE_QSCK1P5;
    stcQspiInit.u32ReleaseTime    = QSPI_QSSN_RELEASE_DELAY_QSCK1P5;
    stcQspiInit.u32IntervalTime   = QSPI_QSSN_INTERVAL_QSCK2;
    (void)QSPI_Init(&stcQspiInit);
}

/**
 * @brief  Reads data from the QSPI memory.
 * @param  [in] u32Addr                 Read start address.
 * @param  [out] pu8ReadBuf             Pointer to the read buffer.
 * @param  [in] u32Size                 Size of the read buffer.
 * @retval int32_t:
 *           - LL_OK: Read succeeded
 *           - LL_ERR_INVD_PARAM: pu8ReadBuf == NULL or u32Size == 0U
 */
int32_t QSPI_FLASH_Read(uint32_t u32Addr, uint8_t *pu8ReadBuf, uint32_t u32Size)
{
    uint32_t u32Count = 0U;
    int32_t i32Ret = LL_OK;
    __IO uint8_t *pu8Read;

    u32Addr += QSPI_ROM_BASE;
    if ((NULL == pu8ReadBuf) || (0UL == u32Size) || ((u32Addr + u32Size) > QSPI_ROM_END)) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
#if (QSPI_XIP_FUNC_ENABLE == DDL_ON)
        QSPI_XipModeCmd(QSPI_FLASH_ENTER_XIP_MD, ENABLE);
#endif
        pu8Read = (__IO uint8_t *)u32Addr;
        while (u32Count < u32Size) {
            pu8ReadBuf[u32Count++] = *pu8Read++;
#if (QSPI_XIP_FUNC_ENABLE == DDL_ON)
            if (u32Count == (u32Size - 1U)) {
                QSPI_XipModeCmd(QSPI_FLASH_EXIT_XIP_MD, DISABLE);
            }
#endif
        }
    }

    return i32Ret;
}

/**
 * @brief  Writes data to the QSPI memory.
 * @param  [in] u32Addr                 Write start address.
 * @param  [in] pu8WriteBuf             Pointer to the write buffer.
 * @param  [in] u32Size                 Size of the write buffer.
 * @retval int32_t:
 *           - LL_OK: Write succeeded
 *           - LL_ERR_INVD_PARAM: pu8WriteBuf == NULL or u32Size == 0U
 */
int32_t QSPI_FLASH_Write(uint32_t u32Addr, uint8_t *pu8WriteBuf, uint32_t u32Size)
{
    uint32_t u32TempSize;
    uint8_t u8AddrBuf[4U];
    uint32_t u32AddrOffset = 0U;
    int32_t i32Ret = LL_OK;

    if ((NULL == pu8WriteBuf) || (0UL == u32Size) || ((u32Addr % W25Q64_PAGE_SIZE) != 0U)) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        while (u32Size != 0UL) {
            if (u32Size >= W25Q64_PAGE_SIZE) {
                u32TempSize = W25Q64_PAGE_SIZE;
            } else {
                u32TempSize = u32Size;
            }
            QSPI_FLASH_WriteInstr(W25Q64_WR_ENABLE, NULL, 0U, NULL, 0U);
            QSPI_FLASH_WordToByte(u32Addr, u8AddrBuf);
            QSPI_FLASH_WriteInstr(W25Q64_PAGE_PROGRAM, u8AddrBuf, (QSPI_FLASH_ADDR_WIDTH + 1U),
                                  (uint8_t *)&pu8WriteBuf[u32AddrOffset], u32TempSize);
            i32Ret = QSPI_FLASH_CheckProcessDone(500U);
            if (i32Ret != LL_OK) {
                break;
            }
            u32Addr       += u32TempSize;
            u32AddrOffset += u32TempSize;
            u32Size       -= u32TempSize;
        }
    }

    return i32Ret;
}

/**
 * @brief  Erase sector of the QSPI memory.
 * @param  [in] u32SectorAddr           The start address of the target sector.
 * @retval int32_t:
 *           - LL_OK: No errors occurred
 *           - LL_ERR_TIMEOUT: Erase sector timeout
 */
int32_t QSPI_FLASH_EraseSector(uint32_t u32SectorAddr)
{
    uint8_t u8AddrBuf[4U];

    QSPI_FLASH_WriteInstr(W25Q64_WR_ENABLE, NULL, 0U, NULL, 0U);
    QSPI_FLASH_WordToByte(u32SectorAddr, u8AddrBuf);
    QSPI_FLASH_WriteInstr(W25Q64_SECTOR_ERASE, u8AddrBuf, (QSPI_FLASH_ADDR_WIDTH + 1U), NULL, 0U);
    return QSPI_FLASH_CheckProcessDone(500U);
}

/**
 * @brief  Erase chip of the QSPI memory.
 * @param  None
 * @retval int32_t:
 *           - LL_OK: No errors occurred
 *           - LL_ERR_TIMEOUT: Erase sector timeout
 */
int32_t QSPI_FLASH_EraseChip(void)
{
    QSPI_FLASH_WriteInstr(W25Q64_WR_ENABLE, NULL, 0U, NULL, 0U);
    QSPI_FLASH_WriteInstr(W25Q64_CHIP_ERASE, NULL, 0U, NULL, 0U);
    return QSPI_FLASH_CheckProcessDone(5000U);
}

/**
 * @brief  Get the UID of the QSPI memory.
 * @param  [in] pu8UID                  The UID of the QSPI memory.
 * @retval None
 */
void QSPI_FLASH_GetUniqueID(uint8_t *pu8UID)
{
    uint32_t i;
    uint8_t u8Dummy[4U];

    /* Fill the dummy values */
    for (i = 0UL; i < 4UL; i++) {
        u8Dummy[i] = 0xFFU;
    }
    QSPI_FLASH_ReadInstr(W25Q64_RD_UNIQUE_ID, u8Dummy, 4U, pu8UID, W25Q64_UNIQUE_ID_SIZE);
}

/**
 * @}
 */

/**
 * @}
 */

/**
* @}
*/

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
