/**
 *******************************************************************************
 * @file  i2s/i2s_play_audio/source/main.c
 * @brief Main program of I2S play audio for the Device Driver Library.
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
 * @addtogroup I2s_Play_Audio
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
extern const uint8_t u8SoundBuf[121590];

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
 * @brief  Initializes the I2S and DMA for the board.
 * @param  None
 * @retval int32_t:
 *           - LL_OK: Initializes success
 *           - LL_ERR_UNINIT: Initializes DMA failed
 */
int32_t BSP_I2S_Init(uint32_t u32AudioFreq)
{
    stc_i2s_init_t stcI2sInit;
    int32_t i32Ret;

    /* I2S pins configuration */
    GPIO_SetFunc(BSP_WM8731_I2S_CK_PORT,   BSP_WM8731_I2S_CK_PIN,   BSP_WM8731_I2S_CK_FUNC);
    GPIO_SetFunc(BSP_WM8731_I2S_WS_PORT,   BSP_WM8731_I2S_WS_PIN,   BSP_WM8731_I2S_WS_FUNC);
    GPIO_SetFunc(BSP_WM8731_I2S_SD_PORT,   BSP_WM8731_I2S_SD_PIN,   BSP_WM8731_I2S_SD_FUNC);
    GPIO_SetFunc(BSP_WM8731_I2S_SDIN_PORT, BSP_WM8731_I2S_SDIN_PIN, BSP_WM8731_I2S_SDIN_FUNC);
    GPIO_SetFunc(BSP_WM8731_I2S_MCK_PORT,  BSP_WM8731_I2S_MCK_PIN,  BSP_WM8731_I2S_MCK_FUNC);

    /* Enable I2S clock */
    FCG_Fcg1PeriphClockCmd(BSP_WM8731_I2S_CLK, ENABLE);
    CLK_SetI2SClockSrc(BSP_WM8731_I2S_CLK_CH, BSP_WM8731_I2S_CLK_SRC);
    /* I2S configuration */
    (void)I2S_StructInit(&stcI2sInit);
    stcI2sInit.u32ClockSrc  = I2S_CLK_SRC_PLL;
    stcI2sInit.u32Mode      = I2S_MD_MASTER;
    stcI2sInit.u32Protocol  = I2S_PROTOCOL_PHILLIPS;
    stcI2sInit.u32TransMode = I2S_TRANS_MD_HALF_DUPLEX_TX;
    stcI2sInit.u32AudioFreq = u32AudioFreq;
    stcI2sInit.u32ChWidth   = I2S_CH_LEN_16BIT;
    stcI2sInit.u32DataWidth = I2S_DATA_LEN_16BIT;
    stcI2sInit.u32MCKOutput = I2S_MCK_OUTPUT_ENABLE;
    i32Ret = I2S_Init(BSP_WM8731_I2S_UNIT, &stcI2sInit);
    /* Enable i2s transfer and receive */
    I2S_FuncCmd(BSP_WM8731_I2S_UNIT, I2S_FUNC_TX, ENABLE);

    return i32Ret;
}

/**
 * @brief  Test buffer configuration.
 * @param  None
 * @retval None
 */
static void I2S_PlayAudio(void)
{
    uint32_t u32DataLen;
    const uint16_t *pu16Sound;

    pu16Sound  = (const uint16_t *)(uint32_t)(&u8SoundBuf[0]);
    u32DataLen = sizeof(u8SoundBuf);
    (void)I2S_Trans(BSP_WM8731_I2S_UNIT, pu16Sound, u32DataLen / 2U, 100U);
}

/**
 * @brief  Main function of I2S play audio.
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
    (void)BSP_WM8731_Init(WM8731_INPUT_DEVICE_NONE, WM8731_OUTPUT_DEVICE_BOTH,
                          80U, WM8731_AUDIO_FREQ_32K, WM8731_DATA_WIDTH_16BIT);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            I2S_PlayAudio();
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
