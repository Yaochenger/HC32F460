/**
 *******************************************************************************
 * @file  i2s/i2s_master_slave_int/source/main.c
 * @brief Main program of I2S master/slave interrupt communication for the
 *        Device Driver Library.
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
 * @addtogroup I2s_Master_Slave_Interrupt
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

/* Master or Slave mode */
#define I2S_UNIT_MASTER_SLAVE           (I2S_MD_MASTER)

/* I2S configuration */
#define I2S_UNIT                        (CM_I2S1)
#define I2S_CLK                         (FCG1_PERIPH_I2S1)
#define I2S_CLK_CH                      (CLK_I2S1)
#define I2S_CLK_SRC                     (CLK_PERIPHCLK_PCLK)

#define I2S_SD_INT_SRC                  (INT_SRC_I2S1_TXIRQOUT)
#define I2S_SD_IRQ                      (INT006_IRQn)
#define I2S_SDIN_INT_SRC                (INT_SRC_I2S1_RXIRQOUT)
#define I2S_SDIN_IRQ                    (INT007_IRQn)

/* CK = PA7 */
#define I2S_CK_PORT                     (GPIO_PORT_A)
#define I2S_CK_PIN                      (GPIO_PIN_07)
#define I2S_CK_FUNC                     (GPIO_FUNC_55)
/* WS = PB0 */
#define I2S_WS_PORT                     (GPIO_PORT_B)
#define I2S_WS_PIN                      (GPIO_PIN_00)
#define I2S_WS_FUNC                     (GPIO_FUNC_54)
/* SD = PC4 */
#define I2S_SD_PORT                     (GPIO_PORT_C)
#define I2S_SD_PIN                      (GPIO_PIN_04)
#define I2S_SD_FUNC                     (GPIO_FUNC_52)
/* SDIN = PC5 */
#define I2S_SDIN_PORT                   (GPIO_PORT_C)
#define I2S_SDIN_PIN                    (GPIO_PIN_05)
#define I2S_SDIN_FUNC                   (GPIO_FUNC_53)

/* The data size for transfer and receive */
#define I2S_DATA_LEN                    (512U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint16_t u16WriteBuffer[I2S_DATA_LEN];
static uint16_t u16ReadBuffer[I2S_DATA_LEN];
static uint8_t u8RxCompleteFlag = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  I2S transmit complete callback.
 * @param  None
 * @retval None
 */
static void I2S_TransCompleteCallback(void)
{
    static uint16_t u16TxCnt = 0U;

    if (SET == I2S_GetStatus(I2S_UNIT, I2S_FLAG_TX_ALARM)) {
        I2S_WriteData(I2S_UNIT, (uint32_t)u16WriteBuffer[u16TxCnt++]);
        if (u16TxCnt >= I2S_DATA_LEN) {
            u16TxCnt = 0U;
        }
    }
}

/**
 * @brief  I2S receive complete callback.
 * @param  None
 * @retval None
 */
static void I2S_ReceiveCompleteCallback(void)
{
    static uint16_t u16RxCnt = 0U;

    if (SET == I2S_GetStatus(I2S_UNIT, I2S_FLAG_RX_ALARM)) {
        u16ReadBuffer[u16RxCnt++] = (uint16_t)I2S_ReadData(I2S_UNIT);
        if (u16RxCnt >= I2S_DATA_LEN) {
            u16RxCnt = 0U;
            u8RxCompleteFlag = 1U;
        }
    }
}

/**
 * @brief  Initializes the I2S and DMA for the board.
 * @param  None
 * @retval None
 */
static void I2S_Config(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;
    stc_i2s_init_t stcI2sInit;

    /* I2S pins configuration */
    GPIO_SetFunc(I2S_CK_PORT,   I2S_CK_PIN,   I2S_CK_FUNC);
    GPIO_SetFunc(I2S_WS_PORT,   I2S_WS_PIN,   I2S_WS_FUNC);
    GPIO_SetFunc(I2S_SD_PORT,   I2S_SD_PIN,   I2S_SD_FUNC);
    GPIO_SetFunc(I2S_SDIN_PORT, I2S_SDIN_PIN, I2S_SDIN_FUNC);

    /* Enable I2S clock */
    FCG_Fcg1PeriphClockCmd(I2S_CLK, ENABLE);
    CLK_SetI2SClockSrc(I2S_CLK_CH, I2S_CLK_SRC);
    /* I2S configuration */
    (void)I2S_StructInit(&stcI2sInit);
    stcI2sInit.u32ClockSrc          = I2S_CLK_SRC_PLL;
    stcI2sInit.u32Mode              = I2S_UNIT_MASTER_SLAVE;
    stcI2sInit.u32Protocol          = I2S_PROTOCOL_PHILLIPS;
    stcI2sInit.u32TransMode         = I2S_TRANS_MD_FULL_DUPLEX;
    stcI2sInit.u32AudioFreq         = I2S_AUDIO_FREQ_48K;
    stcI2sInit.u32ChWidth           = I2S_CH_LEN_32BIT;
    stcI2sInit.u32DataWidth         = I2S_DATA_LEN_16BIT;
    stcI2sInit.u32TransFIFOLevel    = I2S_TRANS_LVL1;
    stcI2sInit.u32ReceiveFIFOLevel  = I2S_RECEIVE_LVL1;
    (void)I2S_Init(I2S_UNIT, &stcI2sInit);

    /* SD NVIC configure */
    stcIrqSignConfig.enIntSrc    = I2S_SD_INT_SRC;
    stcIrqSignConfig.enIRQn      = I2S_SD_IRQ;
    stcIrqSignConfig.pfnCallback = &I2S_TransCompleteCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
    /* SDIN NVIC configure */
    stcIrqSignConfig.enIntSrc    = I2S_SDIN_INT_SRC;
    stcIrqSignConfig.enIRQn      = I2S_SDIN_IRQ;
    stcIrqSignConfig.pfnCallback = &I2S_ReceiveCompleteCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    /* Enable i2s interrupt */
    I2S_IntCmd(I2S_UNIT, (I2S_INT_TX | I2S_INT_RX), ENABLE);
    /* Wait for the K2 to be pressed */
#if I2S_UNIT_MASTER_SLAVE == I2S_MD_MASTER
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_2)) {
    }
#endif
    /* Enable i2s transfer and receive */
    I2S_FuncCmd(I2S_UNIT, (I2S_FUNC_TX | I2S_FUNC_RX), ENABLE);
}

/**
 * @brief  Test buffer configuration.
 * @param  None
 * @retval None
 */
static void TestBufferInit(void)
{
    uint16_t i;

    for (i = 0U; i < I2S_DATA_LEN; i++) {
        u16WriteBuffer[i]  = i + 1U;
        u16ReadBuffer[i] = 0U;
    }
}

/**
 * @brief  Main function of I2S master/slave interrupt communication.
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
    /* Test buffer init */
    TestBufferInit();
    /* Configure I2S */
    I2S_Config();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (0U != u8RxCompleteFlag) {
            u8RxCompleteFlag = 0U;
            if (0 == memcmp(u16WriteBuffer, u16ReadBuffer, (uint16_t)(I2S_DATA_LEN << 1U))) {
                BSP_LED_On(LED_BLUE);
                BSP_LED_Off(LED_RED);
            } else {
                BSP_LED_On(LED_RED);
                BSP_LED_Off(LED_BLUE);
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
