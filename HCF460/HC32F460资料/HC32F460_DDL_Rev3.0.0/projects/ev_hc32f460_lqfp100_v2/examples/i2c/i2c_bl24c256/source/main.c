/**
 *******************************************************************************
 * @file  i2c/i2c_at24c02/source/main.c
 * @brief Main program of I2C for the Device Driver Library.
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
 * @addtogroup I2C_Bl24c256
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

/* Define test address for read and write */
#define DATA_TEST_ADDR                  (0x08U)
#define DATA_TEST_LEN                   (128U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8TxBuf[DATA_TEST_LEN];
static uint8_t u8RxBuf[DATA_TEST_LEN];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t i;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* BSP initialization */
    BSP_CLK_Init();
    BSP_LED_Init();

    for (i = 0UL; i < DATA_TEST_LEN; i++) {
        u8TxBuf[i] = (uint8_t)i + 1U;
    }
    (void)memset(u8RxBuf, (int32_t)0x00U, DATA_TEST_LEN);

    /* Initialize I2C peripheral and enable function*/
    if (LL_OK != BSP_24CXX_Init()) {
        /* Failed */
        for (;;) {
            BSP_LED_Toggle(LED_RED);
            DDL_DelayMS(500UL);
        }
    }

    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    /* EEPROM write*/
    if (LL_OK != BSP_24CXX_Write(DATA_TEST_ADDR, u8TxBuf, DATA_TEST_LEN)) {
        /* Failed */
        for (;;) {
            BSP_LED_Toggle(LED_RED);
            DDL_DelayMS(500UL);
        }
    }

    /* 5mS delay for EEPROM*/
    DDL_DelayMS(5UL);
    while (LL_OK != BSP_24CXX_WaitIdle()) {
        ;
    }

    /* EEPROM read*/
    if (LL_OK != BSP_24CXX_Read(DATA_TEST_ADDR, u8RxBuf, DATA_TEST_LEN)) {
        /* Failed */
        for (;;) {
            BSP_LED_Toggle(LED_RED);
            DDL_DelayMS(500UL);
        }
    }

    /* Compare the data */
    for (i = 0UL; i < DATA_TEST_LEN; i++) {
        if (u8TxBuf[i] != u8RxBuf[i]) {
            /* EEPROM page write error*/
            for (;;) {
                BSP_LED_Toggle(LED_RED);
                DDL_DelayMS(500UL);
            }
        }
    }

    /* EEPROM sample success*/
    for (;;) {
        BSP_LED_Toggle(LED_BLUE);
        DDL_DelayMS(500UL);
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
