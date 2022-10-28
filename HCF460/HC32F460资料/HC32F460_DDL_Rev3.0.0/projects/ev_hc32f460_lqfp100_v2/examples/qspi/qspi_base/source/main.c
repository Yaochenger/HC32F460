/**
 *******************************************************************************
 * @file  qspi/qspi_base/source/main.c
 * @brief Main program of QSPI base for the Device Driver Library.
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
 * @addtogroup QSPI_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

/* Test data size */
#define TEST_DATA_SIZE                  (0x1000UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t m_au8WriteData[TEST_DATA_SIZE];
static uint8_t m_au8ReadData[TEST_DATA_SIZE];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Init the test data.
 * @param  None
 * @retval None
 */
static void QSPI_TestDataInit(void)
{
    uint32_t i;

    for (i = 0UL; i < TEST_DATA_SIZE; i++) {
        m_au8WriteData[i] = (uint8_t)(i % 256U);
        m_au8ReadData[i]  = 0U;
    }
}

/**
 * @brief  Check the erase result.
 * @param  None
 * @retval None
 */
static int32_t QSPI_CheckEraseResult(void)
{
    uint32_t i;
    int32_t i32Ret = LL_OK;

    for (i = 0UL; i < TEST_DATA_SIZE; i++) {
        if (0xFFU != m_au8ReadData[i]) {
            i32Ret = LL_ERR;
            break;
        }
    }

    return i32Ret;
}

/**
 * @brief  Check the program result.
 * @param  None
 * @retval None
 */
static int32_t QSPI_CheckProgramResult(void)
{
    uint32_t i;
    int32_t i32Ret = LL_OK;

    for (i = 0UL; i < TEST_DATA_SIZE; i++) {
        if (m_au8ReadData[i] != m_au8WriteData[i]) {
            i32Ret = LL_ERR;
            break;
        }
    }

    return i32Ret;
}

/**
 * @brief  Main function of QSPI base project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t u8UID[W25Q64_UNIQUE_ID_SIZE] = {0U};
    uint32_t u32FlashAddr = 0UL;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configure QSPI */
    QSPI_FLASH_Init();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
    /* Get flash UID */
    QSPI_FLASH_GetUniqueID(u8UID);
    DDL_Printf("UID value: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\r\n",
               u8UID[0], u8UID[1], u8UID[2], u8UID[3], u8UID[4], u8UID[5], u8UID[6], u8UID[7]);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            BSP_LED_Off(LED_RED);
            BSP_LED_Off(LED_BLUE);
            QSPI_TestDataInit();
            /* Erase sector */
            (void)QSPI_FLASH_EraseSector(u32FlashAddr);
            (void)QSPI_FLASH_Read(u32FlashAddr, m_au8ReadData, TEST_DATA_SIZE);
            if (LL_OK == QSPI_CheckEraseResult()) {
                /* Write data */
                (void)QSPI_FLASH_Write(u32FlashAddr, m_au8WriteData, TEST_DATA_SIZE);
                (void)QSPI_FLASH_Read(u32FlashAddr, m_au8ReadData, TEST_DATA_SIZE);
                if (LL_OK == QSPI_CheckProgramResult()) {
                    BSP_LED_On(LED_BLUE);
                    BSP_LED_Off(LED_RED);
                } else {
                    BSP_LED_On(LED_RED);
                    BSP_LED_Off(LED_BLUE);
                }
            } else {
                BSP_LED_On(LED_RED);
                BSP_LED_Off(LED_BLUE);
            }
            /* Flash address offset */
            u32FlashAddr += W25Q64_SECTOR_SIZE;
            if ((u32FlashAddr + TEST_DATA_SIZE) >= W25Q64_MAX_ADDR) {
                u32FlashAddr = 0U;
            }
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
