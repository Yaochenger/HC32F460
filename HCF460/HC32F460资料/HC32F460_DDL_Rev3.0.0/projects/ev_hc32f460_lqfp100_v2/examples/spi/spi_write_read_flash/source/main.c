/**
 *******************************************************************************
 * @file  spi/spi_write_read_flash/source/main.c
 * @brief Main program SPI write read flash for the Device Driver Library.
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
 * @addtogroup SPI_WRITE_READ_FLASH
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

#define SPI_FLASH_DATA_SIZE             (1024UL * 2UL)
#define SPI_FLASH_ADDR                  (0UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void LoadData(void);
static void ClearData(void);
static void VerifyData(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t m_au8WriteData[SPI_FLASH_DATA_SIZE];
static uint8_t m_au8ReadData[SPI_FLASH_DATA_SIZE];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of spi_master_base project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configures the system clock to 200MHz. */
    BSP_CLK_Init();
    /* BSP LED. */
    BSP_LED_Init();
    /* BSP W25Q64. */
    BSP_W25QXX_Init();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    LoadData();
    for (;;) {
        (void)BSP_W25QXX_EraseSector(SPI_FLASH_ADDR);
        (void)BSP_W25QXX_Write(SPI_FLASH_ADDR, m_au8WriteData, SPI_FLASH_DATA_SIZE);
        (void)BSP_W25QXX_Read(SPI_FLASH_ADDR, m_au8ReadData, SPI_FLASH_DATA_SIZE);
        VerifyData();
        ClearData();
        BSP_LED_Toggle(LED_BLUE);
    }
}

/**
 * @brief  Load data for W25Q64 writting.
 * @param  None
 * @retval None
 */
static void LoadData(void)
{
    uint32_t i;
    for (i = 0UL; i < SPI_FLASH_DATA_SIZE; i++) {
        m_au8WriteData[i] = (uint8_t)i;
    }
}

/**
 * @brief  Clears read data.
 * @param  None
 * @retval None
 */
static void ClearData(void)
{
    uint32_t i;
    for (i = 0UL; i < SPI_FLASH_DATA_SIZE; i++) {
        m_au8ReadData[i]  = 0U;
    }
}

/**
 * @brief  Verifies read data.
 * @param  None
 * @retval None
 */
static void VerifyData(void)
{
    uint32_t i;
    for (i = 0UL; i < SPI_FLASH_DATA_SIZE; i++) {
        if (m_au8ReadData[i] != m_au8WriteData[i]) {
            BSP_LED_Off(LED_BLUE);
            BSP_LED_On(LED_RED);
            for (;;) {
                ;
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
