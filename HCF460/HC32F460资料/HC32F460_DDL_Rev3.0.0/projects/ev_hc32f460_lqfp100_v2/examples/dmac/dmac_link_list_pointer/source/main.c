/*******************************************************************************
* @file  dmac/dmac_link_list_pointer/source/main.c
* @brief This example demonstrates DMA linked list pointer function.
@verbatim
  Change Logs:
  Date             Author       Notes
  2022-03-31       CDT          First version
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
 * @addtogroup DMAC_LLP
 * @{
 */
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* DMAC */
#define DMA_UNIT            (CM_DMA1)
#define DMA_CH              (DMA_CH3)
#define DMA_TC              (1UL)
#define DMA_BC              (10UL)
#define DMA_DW              (DMA_DATAWIDTH_32BIT)
#define DMA_INT_SRC         (INT_SRC_DMA1_TC3)
#define DMA_IRQn            (INT000_IRQn)

#define DMA_LLP0_TC         (5UL << DMA_DTCTL_CNT_POS)
#define DMA_LLP0_BC         (1UL << DMA_DTCTL_BLKSIZE_POS)
#define DMA_LLP0_DW         (DMA_DATAWIDTH_16BIT)

#define DMA_LLP1_TC         (2UL << DMA_DTCTL_CNT_POS)
#define DMA_LLP1_BC         (5UL << DMA_DTCTL_BLKSIZE_POS)
#define DMA_LLP1_DW         (DMA_DATAWIDTH_8BIT)

#define DMA_TRIGGER_CH      (AOS_DMA1_3)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static uint8_t m_u8Conut = 0U;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static const uint32_t u32SrcBuf0[10] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
static uint32_t u32DestBuf0[10] = {0};

static const uint16_t u16SrcBuf1[5] = {21, 22, 23, 24, 25};
static uint16_t u16DestBuf1[5] = {0};

static const uint8_t u8SrcBuf2[10] = {31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
static uint8_t u8DestBuf2[10] = {0};

static stc_dma_llp_descriptor_t stcLlpDesc[2] = {0};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  DMA basic and llp function init
 * @param  None
 * @retval None
 */
static void DmaInit(void)
{
    stc_dma_init_t stcDmaInit;
    stc_dma_llp_init_t stcDmaLlpInit;

    AOS_SetTriggerEventSrc(DMA_TRIGGER_CH, EVT_SRC_AOS_STRG);

    (void)DMA_StructInit(&stcDmaInit);

    stcDmaInit.u32IntEn      = DMA_INT_ENABLE;
    stcDmaInit.u32BlockSize  = DMA_BC;
    stcDmaInit.u32TransCount = DMA_TC;
    stcDmaInit.u32DataWidth  = DMA_DW;
    stcDmaInit.u32DestAddr   = (uint32_t)(&u32DestBuf0[0]);
    stcDmaInit.u32SrcAddr    = (uint32_t)(&u32SrcBuf0[0]);
    stcDmaInit.u32SrcAddrInc = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;

    (void)DMA_Init(DMA_UNIT, DMA_CH, &stcDmaInit);

    stcLlpDesc[0].SARx  = (uint32_t)&u16SrcBuf1[0];
    stcLlpDesc[0].DARx  = (uint32_t)&u16DestBuf1[0];
    stcLlpDesc[0].DTCTLx = DMA_LLP0_TC | DMA_LLP0_BC;
    stcLlpDesc[0].LLPx  = (uint32_t)&stcLlpDesc[1];
    stcLlpDesc[0].CHCTLx = DMA_SRC_ADDR_INC | DMA_DEST_ADDR_INC | DMA_LLP0_DW |  \
                           DMA_LLP_ENABLE   | DMA_INT_ENABLE | DMA_LLP_WAIT;

    stcLlpDesc[1].SARx  = (uint32_t)&u8SrcBuf2[0];
    stcLlpDesc[1].DARx  = (uint32_t)&u8DestBuf2[0];
    stcLlpDesc[1].DTCTLx = DMA_LLP1_TC | DMA_LLP1_BC;
    stcLlpDesc[1].CHCTLx = DMA_SRC_ADDR_INC | DMA_DEST_ADDR_INC | DMA_LLP1_DW |  \
                           DMA_LLP_DISABLE  | DMA_INT_ENABLE;

    (void)DMA_LlpStructInit(&stcDmaLlpInit);

    stcDmaLlpInit.u32State = DMA_LLP_ENABLE;
    stcDmaLlpInit.u32Mode  = DMA_LLP_WAIT;
    stcDmaLlpInit.u32Addr  = (uint32_t)&stcLlpDesc[0];

    (void)DMA_LlpInit(DMA_UNIT, DMA_CH, &stcDmaLlpInit);
}

/**
 * @brief  DMA ch.3 transfer complete IRQ callback
 * @param  None
 * @retval None
 */
static void DMA1_CH3_TransEnd_IrqCallback(void)
{
    m_u8Conut++;
    DMA_ClearTransCompleteStatus(DMA_UNIT, DMA_FLAG_TC_CH3);
}

/**
 * @brief  DMA basic function interrupt init
 * @param  None
 * @retval None
 */
static void DmaIntInit(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;

    stcIrqSignConfig.enIntSrc   = DMA_INT_SRC;
    stcIrqSignConfig.enIRQn     = DMA_IRQn;
    stcIrqSignConfig.pfnCallback = &DMA1_CH3_TransEnd_IrqCallback;

    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    DMA_ClearTransCompleteStatus(DMA_UNIT, DMA_FLAG_TC_CH3);

    /* NVIC setting */
    NVIC_ClearPendingIRQ(DMA_IRQn);
    NVIC_SetPriority(DMA_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(DMA_IRQn);
}

/**
 * @brief  Main function of DMAC project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    int32_t i32Ret1;
    int32_t i32Ret2;
    int32_t i32Ret3;

    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System clock init */
    BSP_CLK_Init();
    /* LED initialization */
    BSP_LED_Init();

    /* Enable DMA/AOS FCG */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DMA1 | FCG0_PERIPH_AOS, ENABLE);

    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Config DMA */
    DmaInit();
    /* DMA interrupt config */
    DmaIntInit();
    /* DMA module enable */
    DMA_Cmd(DMA_UNIT, ENABLE);

    /* DMA channel enable */
    (void)DMA_ChCmd(DMA_UNIT, DMA_CH, ENABLE);

    /* 1st trigger for DMA */
    AOS_SW_Trigger();

    /* There will enter three times interrupt */
    while (3U != m_u8Conut) {
        if (SET == DMA_GetTransCompleteStatus(DMA_UNIT, DMA_FLAG_BTC_CH3)) {
            DMA_ClearTransCompleteStatus(DMA_UNIT, DMA_FLAG_BTC_CH3);
            AOS_SW_Trigger();
        }
    }

    i32Ret1 = memcmp(u32DestBuf0, u32SrcBuf0, sizeof(u32SrcBuf0));

    i32Ret2 = memcmp(u16DestBuf1, u16SrcBuf1, sizeof(u16SrcBuf1));

    i32Ret3 = memcmp(u8DestBuf2, u8SrcBuf2, sizeof(u8SrcBuf2));

    if ((0 == i32Ret1) && (0 == i32Ret2) && (0 == i32Ret3)) {
        /* LED blue, as expected */
        BSP_LED_On(LED_BLUE);
    } else {
        /* LED red */
        BSP_LED_On(LED_RED);
    }
    for (;;) {
        ;
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
