/**
 *******************************************************************************
 * @file  spi/spi_dma/source/main.c
 * @brief Main program SPI tx/rx dma for the Device Driver Library.
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
 * @addtogroup SPI_DMA
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

/* Configuration for Example */
#define EXAMPLE_SPI_MASTER_SLAVE        (SPI_MASTER)
#define EXAMPLE_SPI_BUF_LEN             (128UL)

/* SPI definition */
#define SPI_UNIT                        (CM_SPI1)
#define SPI_CLK                         (FCG1_PERIPH_SPI1)
#define SPI_TX_EVT_SRC                  (EVT_SRC_SPI1_SPTI)
#define SPI_RX_EVT_SRC                  (EVT_SRC_SPI1_SPRI)

/* DMA definition */
#define DMA_UNIT                        (CM_DMA1)
#define DMA_CLK                         (FCG0_PERIPH_DMA1 | FCG0_PERIPH_AOS)
#define DMA_TX_CH                       (DMA_CH0)
#define DMA_TX_TRIG_CH                  (AOS_DMA1_0)

#define DMA_RX_CH                       (DMA_CH1)
#define DMA_RX_INT_CH                   (DMA_INT_TC_CH1)
#define DMA_RX_TRIG_CH                  (AOS_DMA1_1)
#define DMA_RX_INT_SRC                  (INT_SRC_DMA1_TC1)
#define DMA_RX_IRQ_NUM                  (INT006_IRQn)

/* SS = PA7 */
#define SPI_SS_PORT                     (GPIO_PORT_A)
#define SPI_SS_PIN                      (GPIO_PIN_07)
#define SPI_SS_FUNC                     (GPIO_FUNC_42)
/* SCK = PA8 */
#define SPI_SCK_PORT                    (GPIO_PORT_A)
#define SPI_SCK_PIN                     (GPIO_PIN_08)
#define SPI_SCK_FUNC                    (GPIO_FUNC_43)
/* MOSI = PB0 */
#define SPI_MOSI_PORT                   (GPIO_PORT_B)
#define SPI_MOSI_PIN                    (GPIO_PIN_00)
#define SPI_MOSI_FUNC                   (GPIO_FUNC_40)
/* MISO = PC5 */
#define SPI_MISO_PORT                   (GPIO_PORT_C)
#define SPI_MISO_PIN                    (GPIO_PIN_05)
#define SPI_MISO_FUNC                   (GPIO_FUNC_41)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static char u8TxBuf[EXAMPLE_SPI_BUF_LEN] = "SPI Master/Slave example: Communication between two boards!";
static char u8RxBuf[EXAMPLE_SPI_BUF_LEN];
static __IO en_flag_status_t enRxCompleteFlag = RESET;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  DMA transmit complete callback.
 * @param  None
 * @retval None
 */
static void DMA_TransCompleteCallback(void)
{
    enRxCompleteFlag = SET;
    DMA_ClearTransCompleteStatus(DMA_UNIT, DMA_RX_INT_CH);
}

/**
 * @brief  SPI configurate.
 * @param  None
 * @retval None
 */
static void SPI_Config(void)
{
    stc_spi_init_t stcSpiInit;
    stc_dma_init_t stcDmaInit;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Configure Port */
    GPIO_SetFunc(SPI_SS_PORT,   SPI_SS_PIN,   SPI_SS_FUNC);
    GPIO_SetFunc(SPI_SCK_PORT,  SPI_SCK_PIN,  SPI_SCK_FUNC);
    GPIO_SetFunc(SPI_MOSI_PORT, SPI_MOSI_PIN, SPI_MOSI_FUNC);
    GPIO_SetFunc(SPI_MISO_PORT, SPI_MISO_PIN, SPI_MISO_FUNC);

    /* Configuration SPI */
    FCG_Fcg1PeriphClockCmd(SPI_CLK, ENABLE);
    SPI_StructInit(&stcSpiInit);
    stcSpiInit.u32WireMode          = SPI_4_WIRE;
    stcSpiInit.u32TransMode         = SPI_FULL_DUPLEX;
    stcSpiInit.u32MasterSlave       = EXAMPLE_SPI_MASTER_SLAVE;
    stcSpiInit.u32Parity            = SPI_PARITY_INVD;
    stcSpiInit.u32SpiMode           = SPI_MD_1;
    stcSpiInit.u32BaudRatePrescaler = SPI_BR_CLK_DIV64;
    stcSpiInit.u32DataBits          = SPI_DATA_SIZE_8BIT;
    stcSpiInit.u32FirstBit          = SPI_FIRST_MSB;
    stcSpiInit.u32FrameLevel        = SPI_1_FRAME;
    (void)SPI_Init(SPI_UNIT, &stcSpiInit);

    /* DMA configuration */
    FCG_Fcg0PeriphClockCmd(DMA_CLK, ENABLE);
    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32BlockSize  = 1UL;
    stcDmaInit.u32TransCount = EXAMPLE_SPI_BUF_LEN;
    stcDmaInit.u32DataWidth  = DMA_DATAWIDTH_8BIT;
    /* Configure TX */
    stcDmaInit.u32SrcAddrInc  = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_FIX;
    stcDmaInit.u32SrcAddr     = (uint32_t)(&u8TxBuf[0]);
    stcDmaInit.u32DestAddr    = (uint32_t)(&SPI_UNIT->DR);
    if (LL_OK != DMA_Init(DMA_UNIT, DMA_TX_CH, &stcDmaInit)) {
        for (;;) {
        }
    }
    AOS_SetTriggerEventSrc(DMA_TX_TRIG_CH, SPI_TX_EVT_SRC);
    /* Configure RX */
    stcDmaInit.u32IntEn       = DMA_INT_ENABLE;
    stcDmaInit.u32SrcAddrInc  = DMA_SRC_ADDR_FIX;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;
    stcDmaInit.u32SrcAddr     = (uint32_t)(&SPI_UNIT->DR);
    stcDmaInit.u32DestAddr    = (uint32_t)(&u8RxBuf[0]);
    if (LL_OK != DMA_Init(DMA_UNIT, DMA_RX_CH, &stcDmaInit)) {
        for (;;) {
        }
    }
    AOS_SetTriggerEventSrc(DMA_RX_TRIG_CH, SPI_RX_EVT_SRC);

    /* DMA receive NVIC configure */
    stcIrqSignConfig.enIntSrc    = DMA_RX_INT_SRC;
    stcIrqSignConfig.enIRQn      = DMA_RX_IRQ_NUM;
    stcIrqSignConfig.pfnCallback = &DMA_TransCompleteCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    /* Enable DMA and channel */
    DMA_Cmd(DMA_UNIT, ENABLE);
    DMA_ChCmd(DMA_UNIT, DMA_TX_CH, ENABLE);
    DMA_ChCmd(DMA_UNIT, DMA_RX_CH, ENABLE);
}

/**
 * @brief  SPI configurate.
 * @param  None
 * @retval None
 */
static void DMA_ReloadConfig(void)
{
    DMA_SetSrcAddr(DMA_UNIT, DMA_TX_CH, (uint32_t)(&u8TxBuf[0]));
    DMA_SetTransCount(DMA_UNIT, DMA_TX_CH, EXAMPLE_SPI_BUF_LEN);
    DMA_SetDestAddr(DMA_UNIT, DMA_RX_CH, (uint32_t)(&u8RxBuf[0]));
    DMA_SetTransCount(DMA_UNIT, DMA_RX_CH, EXAMPLE_SPI_BUF_LEN);
    /* Enable DMA channel */
    DMA_ChCmd(DMA_UNIT, DMA_TX_CH, ENABLE);
    DMA_ChCmd(DMA_UNIT, DMA_RX_CH, ENABLE);
}

/**
 * @brief  Main function of SPI tx/rx dma project
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
    /* Configure SPI */
    SPI_Config();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        /* Wait key trigger in master mode */
#if (EXAMPLE_SPI_MASTER_SLAVE == SPI_MASTER)
        while (RESET == BSP_KEY_GetStatus(BSP_KEY_2)) {
        }
#endif
        enRxCompleteFlag = RESET;
        memset(u8RxBuf, 0, EXAMPLE_SPI_BUF_LEN);
        DMA_ReloadConfig();
        /* Enable SPI */
        SPI_Cmd(SPI_UNIT, ENABLE);
        /* Waiting for completion of reception */
        while (RESET == enRxCompleteFlag) {
        }
        /* Disable SPI */
        SPI_Cmd(SPI_UNIT, DISABLE);

        /* Compare u8TxBuf and u8RxBuf */
        if (0 == memcmp(u8TxBuf, u8RxBuf, EXAMPLE_SPI_BUF_LEN)) {
            BSP_LED_On(LED_BLUE);
            BSP_LED_Off(LED_RED);
        } else {
            BSP_LED_On(LED_RED);
            BSP_LED_Off(LED_BLUE);
        }
#if (EXAMPLE_SPI_MASTER_SLAVE == SPI_MASTER)
        /* Wait for the slave to be ready */
        DDL_DelayMS(10U);
#endif
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
