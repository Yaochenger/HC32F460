/**
 *******************************************************************************
 * @file  sdioc/sdioc_sd/source/main.c
 * @brief Main program of SDIOC SD card for the Device Driver Library.
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
 * @addtogroup SDIOC_SD_Card
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

/* SD transfer mode */
#define SD_TRANS_MD_POLLING             (0U)
#define SD_TRANS_MD_INT                 (1U)
#define SD_TRANS_MD_DMA                 (2U)
/* Populate the following macro with an value, reference "SD transfer mode" */
#define SD_TRANS_MD                     (SD_TRANS_MD_POLLING)

/* SDIOC DMA configuration define */
#define SDIOC_DMA_UNIT                  (CM_DMA1)
#define SDIOC_DMA_CLK                   (FCG0_PERIPH_DMA1 | FCG0_PERIPH_AOS)
#define SDIOC_DMA_TX_CH                 (DMA_CH0)
#define SDIOC_DMA_RX_CH                 (DMA_CH1)
#define SDIOC_DMA_TX_TRIG_CH            (AOS_DMA1_0)
#define SDIOC_DMA_RX_TRIG_CH            (AOS_DMA1_1)
#define SDIOC_DMA_TX_TRIG_SRC           (EVT_SRC_SDIOC1_DMAW)
#define SDIOC_DMA_RX_TRIG_SRC           (EVT_SRC_SDIOC1_DMAR)

/* SDIOC configuration define */
#define SDIOC_SD_UINT                   (CM_SDIOC1)
#define SDIOC_SD_CLK                    (FCG1_PERIPH_SDIOC1)
#define SIDOC_SD_INT_SRC                (INT_SRC_SDIOC1_SD)
#define SIDOC_SD_IRQ                    (INT006_IRQn)
/* CK = PC12 */
#define SDIOC_CK_PORT                   (GPIO_PORT_C)
#define SDIOC_CK_PIN                    (GPIO_PIN_12)
/* CMD = PD02 */
#define SDIOC_CMD_PORT                  (GPIO_PORT_D)
#define SDIOC_CMD_PIN                   (GPIO_PIN_02)
/* D0 = PC08 */
#define SDIOC_D0_PORT                   (GPIO_PORT_C)
#define SDIOC_D0_PIN                    (GPIO_PIN_08)
/* D1 = PC09 */
#define SDIOC_D1_PORT                   (GPIO_PORT_C)
#define SDIOC_D1_PIN                    (GPIO_PIN_09)
/* D2 = PC10 */
#define SDIOC_D2_PORT                   (GPIO_PORT_C)
#define SDIOC_D2_PIN                    (GPIO_PIN_10)
/* D3 = PC11 */
#define SDIOC_D3_PORT                   (GPIO_PORT_C)
#define SDIOC_D3_PIN                    (GPIO_PIN_11)
/* CD = PE14 */
#define SDIOC_CD_PORT                   (GPIO_PORT_E)
#define SDIOC_CD_PIN                    (GPIO_PIN_14)

/* SD card read/write/erase */
#define SD_CARD_BLK_SIZE                (512UL)
#define SD_CARD_BLK_NUM                 (10U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static stc_sd_handle_t SdHandle;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8WriteBlocks[SD_CARD_BLK_SIZE * SD_CARD_BLK_NUM];
static uint8_t u8ReadBlocks[SD_CARD_BLK_SIZE * SD_CARD_BLK_NUM];
#if SD_TRANS_MD != SD_TRANS_MD_POLLING
__IO static uint8_t u8TxCompleteFlag = 0U, u8RxCompleteFlag = 0U, u8TxRxErrorFlag = 0U;
#endif

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
#if SD_TRANS_MD != SD_TRANS_MD_POLLING
/**
 * @brief  SDIOC transfer complete interrupt callback function.
 * @param  None
 * @retval None
 */
static void SdCard_TransCompleteIrqCallback(void)
{
    SD_IRQHandler(&SdHandle);
}

/**
 * @brief  SDIOC TX complete callback function.
 * @param  None
 * @retval None
 */
void SD_TxCompleteCallback(stc_sd_handle_t *handle)
{
    u8TxCompleteFlag = 1U;
}

/**
 * @brief  SDIOC RX complete callback function.
 * @param  None
 * @retval None
 */
void SD_RxCompleteCallback(stc_sd_handle_t *handle)
{
    u8RxCompleteFlag = 1U;
}

/**
 * @brief  SDIOC error callback function.
 * @param  None
 * @retval None
 */
void SD_ErrorCallback(stc_sd_handle_t *handle)
{
    u8TxRxErrorFlag = 1U;
}
#endif

#if SD_TRANS_MD == SD_TRANS_MD_DMA
/**
 * @brief  Initializes the SD DMA.
 * @param  None
 * @retval None
 */
static void SdCard_DMAInit(void)
{
    stc_dma_init_t stcDmaInit;

    /* Enable DMA and AOS clock */
    FCG_Fcg0PeriphClockCmd(SDIOC_DMA_CLK, ENABLE);

    (void)DMA_StructInit(&stcDmaInit);
    /* Configure SD DMA Transfer */
    stcDmaInit.u32IntEn         = DMA_INT_DISABLE;
    stcDmaInit.u32DataWidth     = DMA_DATAWIDTH_32BIT;
    stcDmaInit.u32SrcAddrInc    = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc   = DMA_DEST_ADDR_FIX;
    if (LL_OK != DMA_Init(SDIOC_DMA_UNIT, SDIOC_DMA_TX_CH, &stcDmaInit)) {
        for (;;) {
        }
    }
    /* Configure SD DMA Receive */
    stcDmaInit.u32SrcAddrInc    = DMA_SRC_ADDR_FIX;
    stcDmaInit.u32DestAddrInc   = DMA_DEST_ADDR_INC;
    if (LL_OK != DMA_Init(SDIOC_DMA_UNIT, SDIOC_DMA_RX_CH, &stcDmaInit)) {
        for (;;)
        {}
    }

    AOS_SetTriggerEventSrc(SDIOC_DMA_TX_TRIG_CH, SDIOC_DMA_TX_TRIG_SRC);
    AOS_SetTriggerEventSrc(SDIOC_DMA_RX_TRIG_CH, SDIOC_DMA_RX_TRIG_SRC);
    /* Enable DMA */
    DMA_Cmd(SDIOC_DMA_UNIT, ENABLE);
}
#endif

/**
 * @brief  SD card configuration.
 * @param  None
 * @retval None
 */
static void SdCard_Config(void)
{
#if SD_TRANS_MD != SD_TRANS_MD_POLLING
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Interrupt configuration */
    stcIrqSignConfig.enIntSrc    = SIDOC_SD_INT_SRC;
    stcIrqSignConfig.enIRQn      = SIDOC_SD_IRQ;
    stcIrqSignConfig.pfnCallback = &SdCard_TransCompleteIrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
#endif

    /* Enable SDIOC clock */
    FCG_Fcg1PeriphClockCmd(SDIOC_SD_CLK, ENABLE);
    /* SDIOC pins configuration */
    GPIO_SetFunc(SDIOC_CD_PORT,  SDIOC_CD_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_CK_PORT,  SDIOC_CK_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_CMD_PORT, SDIOC_CMD_PIN, GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D0_PORT,  SDIOC_D0_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D1_PORT,  SDIOC_D1_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D2_PORT,  SDIOC_D2_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D3_PORT,  SDIOC_D3_PIN,  GPIO_FUNC_9);

    /* Configure structure initialization */
    SdHandle.SDIOCx                     = SDIOC_SD_UINT;
    SdHandle.stcSdiocInit.u32Mode       = SDIOC_MD_SD;
    SdHandle.stcSdiocInit.u8CardDetect  = SDIOC_CARD_DETECT_CD_PIN_LVL;
    SdHandle.stcSdiocInit.u8SpeedMode   = SDIOC_SPEED_MD_HIGH;
    SdHandle.stcSdiocInit.u8BusWidth    = SDIOC_BUS_WIDTH_4BIT;
    SdHandle.stcSdiocInit.u16ClockDiv   = SDIOC_CLK_DIV2;
#if SD_TRANS_MD == SD_TRANS_MD_DMA
    SdCard_DMAInit();
    SdHandle.DMAx      = SDIOC_DMA_UNIT;
    SdHandle.u8DmaTxCh = SDIOC_DMA_TX_CH;
    SdHandle.u8DmaRxCh = SDIOC_DMA_RX_CH;
#else
    SdHandle.DMAx    = NULL;
#endif

    /* Reset and init SDIOC */
    if (LL_OK != SDIOC_SWReset(SdHandle.SDIOCx, SDIOC_SW_RST_ALL)) {
        DDL_Printf("Reset SDIOC failed!\r\n");
    } else if (SET != SDIOC_GetInsertStatus(SdHandle.SDIOCx)) {
        DDL_Printf("No SD card insertion!\r\n");
    } else if (LL_OK != SD_Init(&SdHandle)) {
        DDL_Printf("SD card initialize failed!\r\n");
    } else {
    }
}

/**
 * @brief  SD card erase.
 * @param  None
 * @retval int32_t:
 *           - LL_OK: SD card erase success
 *           - LL_ERR: SD card erase error
 */
static int32_t SdCard_Erase(void)
{
    uint32_t i;
    int32_t i32Ret = LL_OK;

    /* Initialize read/write blocks */
    (void)memset(u8ReadBlocks, 0x20, (SD_CARD_BLK_SIZE * SD_CARD_BLK_NUM));
    /* Erase SD card */
    if (LL_OK != SD_Erase(&SdHandle, 0UL, SD_CARD_BLK_NUM)) {
        i32Ret = LL_ERR;
    }

    /* Read SD card */
#if SD_TRANS_MD == SD_TRANS_MD_POLLING
    if (LL_OK != SD_ReadBlocks(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8ReadBlocks, 2000UL)) {
        i32Ret = LL_ERR;
    }
#elif SD_TRANS_MD == SD_TRANS_MD_INT
    u8RxCompleteFlag = 0U;
    if (LL_OK != SD_ReadBlocks_INT(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8ReadBlocks)) {
        i32Ret = LL_ERR;
    }
    /* Wait for transfer completed */
    while ((0U == u8RxCompleteFlag) && (0U == u8TxRxErrorFlag)) {
    }
#else
    u8RxCompleteFlag = 0U;
    if (LL_OK != SD_ReadBlocks_DMA(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8ReadBlocks)) {
        i32Ret = LL_ERR;
    }
    /* Wait for transfer completed */
    while ((0U == u8RxCompleteFlag) && (0U == u8TxRxErrorFlag)) {
    }
#endif

    /* Check whether data value is OxFF or 0x00 after erase SD card */
    for (i = 0UL; i < (SD_CARD_BLK_SIZE * SD_CARD_BLK_NUM); i++) {
        if ((0x00U != u8ReadBlocks[i]) && (0xFFU != u8ReadBlocks[i])) {
            i32Ret = LL_ERR;
            break;
        }
    }
#if SD_TRANS_MD != SD_TRANS_MD_POLLING
    if (0U != u8TxRxErrorFlag) {
        i32Ret = LL_ERR;
    }
#endif
    if (LL_OK != i32Ret) {
        DDL_Printf("SD card erase failed!\r\n");
    }

    return i32Ret;
}

/**
 * @brief  SD card multi-block read/write.
 * @param  None
 * @retval int32_t:
 *           - LL_OK: SD card multi-block read/write success
 *           - LL_ERR: SD card multi-block read/write error
 */
static int32_t SdCard_RdWrMultiBlock(void)
{
    int32_t i32Ret = LL_OK;

    /* Initialize read/write blocks */
    (void)memset(u8WriteBlocks, 0x20, (SD_CARD_BLK_SIZE * SD_CARD_BLK_NUM));
    (void)memset(u8ReadBlocks,  0, (SD_CARD_BLK_SIZE * SD_CARD_BLK_NUM));
    /* Write SD card */
#if SD_TRANS_MD == SD_TRANS_MD_POLLING
    if (LL_OK != SD_WriteBlocks(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8WriteBlocks, 2000U)) {
        i32Ret = LL_ERR;
    }
#elif SD_TRANS_MD == SD_TRANS_MD_INT
    u8TxCompleteFlag = 0U;
    if (LL_OK != SD_WriteBlocks_INT(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8WriteBlocks)) {
        i32Ret = LL_ERR;
    }
    /* Wait for transfer completed */
    while ((0U == u8TxCompleteFlag) && (0U == u8TxRxErrorFlag)) {
    }
#else
    u8TxCompleteFlag = 0U;
    if (LL_OK != SD_WriteBlocks_DMA(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8WriteBlocks)) {
        i32Ret = LL_ERR;
    }
    /* Wait for transfer completed */
    while ((0U == u8TxCompleteFlag) && (0U == u8TxRxErrorFlag)) {
    }
#endif

    /* Read SD card */
#if SD_TRANS_MD == SD_TRANS_MD_POLLING
    if (LL_OK != SD_ReadBlocks(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8ReadBlocks, 2000UL)) {
        i32Ret = LL_ERR;
    }
#elif SD_TRANS_MD == SD_TRANS_MD_INT
    u8RxCompleteFlag = 0U;
    if (LL_OK != SD_ReadBlocks_INT(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8ReadBlocks)) {
        i32Ret = LL_ERR;
    }
    /* Wait for transfer completed */
    while ((0U == u8RxCompleteFlag) && (0U == u8TxRxErrorFlag)) {
    }
#else
    u8RxCompleteFlag = 0U;
    if (LL_OK != SD_ReadBlocks_DMA(&SdHandle, 0UL, SD_CARD_BLK_NUM, (uint8_t *)u8ReadBlocks)) {
        i32Ret = LL_ERR;
    }
    /* Wait for transfer completed */
    while ((0U == u8RxCompleteFlag) && (0U == u8TxRxErrorFlag)) {
    }
#endif

    /* Check data value */
    if (0 != memcmp(u8WriteBlocks, u8ReadBlocks, (SD_CARD_BLK_SIZE * SD_CARD_BLK_NUM))) {
        i32Ret = LL_ERR;
    }
#if SD_TRANS_MD != SD_TRANS_MD_POLLING
    if (0U != u8TxRxErrorFlag) {
        i32Ret = LL_ERR;
    }
#endif
    if (LL_OK != i32Ret) {
        DDL_Printf("SD card multi-block read/write failed!\r\n");
    }

    return i32Ret;
}

/**
 * @brief  Main function of SDIOC SD card.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    int32_t i32EraseRet;
    int32_t i32MultiBlockRet;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configure SD Card */
    SdCard_Config();
    /* Erase/single-block/multi-block test */
    i32EraseRet      = SdCard_Erase();
    i32MultiBlockRet = SdCard_RdWrMultiBlock();
    if ((LL_OK != i32EraseRet) || (LL_OK != i32MultiBlockRet)) {
        /* Test failed */
        BSP_LED_On(LED_RED);
        BSP_LED_Off(LED_BLUE);
    } else {
        /* Test success */
        BSP_LED_On(LED_BLUE);
        BSP_LED_Off(LED_RED);
    }
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

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
