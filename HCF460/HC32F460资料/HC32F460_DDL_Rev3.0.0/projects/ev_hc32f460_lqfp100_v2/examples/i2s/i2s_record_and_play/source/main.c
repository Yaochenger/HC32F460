/**
 *******************************************************************************
 * @file  i2s/i2s_record_and_play/source/main.c
 * @brief Main program of I2S record and play for the Device Driver Library.
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
 * @addtogroup I2S_Record_And_Play
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

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint32_t u32TransBuf[2][BSP_WM8731_DMA_BLK_LEN];
static uint32_t u32ReceiveBuf[2][BSP_WM8731_DMA_BLK_LEN];
static uint8_t u8TransCompleteFlag = 0U;
static uint8_t u8ReceiveCompleteFlag = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  DMA transfer complete callback.
 * @param  None
 * @retval None
 */
void BSP_WM8731_TransCompleteCallBack(void)
{
    if (0U != u8TransCompleteFlag) {
        BSP_WM8731_Play(&u32TransBuf[1][0], BSP_WM8731_DMA_BLK_LEN);
        u8TransCompleteFlag = 0U;
    } else {
        BSP_WM8731_Play(&u32TransBuf[0][0], BSP_WM8731_DMA_BLK_LEN);
        u8TransCompleteFlag = 1U;
    }
    DMA_ClearTransCompleteStatus(BSP_WM8731_DMA_SDIN_UNIT, BSP_WM8731_DMA_SD_INT_CH);
}

/**
 * @brief  DMA receive complete callback.
 * @param  None
 * @retval None
 */
void BSP_WM8731_ReceiveCompleteCallBack(void)
{
    if (0U != u8ReceiveCompleteFlag) {
        BSP_WM8731_Record(&u32ReceiveBuf[0][0], BSP_WM8731_DMA_BLK_LEN);
        (void)memcpy((uint8_t *)&u32TransBuf[1][0], (uint8_t *)&u32ReceiveBuf[1][0], (BSP_WM8731_DMA_BLK_LEN * 4U));
        u8ReceiveCompleteFlag = 0U;
    } else {
        BSP_WM8731_Record(&u32ReceiveBuf[1][0], BSP_WM8731_DMA_BLK_LEN);
        (void)memcpy((uint8_t *)&u32TransBuf[0][0], (uint8_t *)&u32ReceiveBuf[0][0], (BSP_WM8731_DMA_BLK_LEN * 4U));
        u8ReceiveCompleteFlag = 1U;
    }
    DMA_ClearTransCompleteStatus(BSP_WM8731_DMA_SD_UNIT, BSP_WM8731_DMA_SDIN_INT_CH);
}

/**
 * @brief  Main function of I2S record and play.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Configure WM8731 */
    (void)BSP_WM8731_Init(WM8731_INPUT_DEVICE_MICROPHONE, WM8731_OUTPUT_DEVICE_BOTH,
                          80U, WM8731_AUDIO_FREQ_32K, WM8731_DATA_WIDTH_32BIT);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (RESET != BSP_KEY_GetStatus(BSP_KEY_2)) {
            BSP_LED_On(LED_BLUE);
            u8TransCompleteFlag = 0U;
            u8ReceiveCompleteFlag = 0U;
            BSP_WM8731_Record(&u32ReceiveBuf[0][0], BSP_WM8731_DMA_BLK_LEN);
            BSP_WM8731_Play(&u32TransBuf[0][0], BSP_WM8731_DMA_BLK_LEN);
        }
        if (RESET != BSP_KEY_GetStatus(BSP_KEY_3)) {
            BSP_WM8731_Stop();
            BSP_LED_Off(LED_BLUE);
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
