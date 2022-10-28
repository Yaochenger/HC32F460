/*******************************************************************************
* @file  dmac/dmac_repeat/source/main.c
* @brief This example demonstrates DMA repeat transfer function.
@verbatim
  Change Logs:
  Date             Author          Notes
  2022-03-31       CDT         First version
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
 * @addtogroup DMAC_Repeat
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
#define DMA_MX_CH           (DMA_MX_CH3)
#define DMA_TC              (20UL)
#define DMA_BC              (1UL)
#define DMA_DW              (DMA_DATAWIDTH_32BIT)
#define DMA_INT_SRC         (INT_SRC_DMA1_TC3)
#define DMA_IRQn            (INT000_IRQn)

#define DMA_SRC_RPT_SIZE    (2U)

#define DMA_TRIGGER_CH      (AOS_DMA1_3)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
__IO static en_flag_status_t m_u8DmaTcEnd = RESET;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void DmaInit(void);
void DMA1_CH3_TransEnd_IrqCallback(void);
static void DmaIntInit(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static const uint32_t u32SrcBuf[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                       11, 12, 13, 14, 15, 16, 17, 18,
                                       19, 20
                                      };
static uint32_t u32DestBuf[20] = {0};
static uint32_t u32ExpectDestBufData[20] = {1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
                                            1, 2, 1, 2, 1, 2, 1, 2, 1, 2
                                           };

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  DMA basic and repeat function init
 * @param  None
 * @retval None
 */
static void DmaInit(void)
{
    stc_dma_init_t stcDmaInit;
    stc_dma_repeat_init_t stcDmaRepeatInit;

    AOS_SetTriggerEventSrc(DMA_TRIGGER_CH, EVT_SRC_AOS_STRG);

    (void)DMA_StructInit(&stcDmaInit);

    stcDmaInit.u32IntEn      = DMA_INT_ENABLE;
    stcDmaInit.u32BlockSize  = DMA_BC;
    stcDmaInit.u32TransCount = DMA_TC;
    stcDmaInit.u32DataWidth  = DMA_DW;
    stcDmaInit.u32DestAddr   = (uint32_t)(&u32DestBuf[0]);
    stcDmaInit.u32SrcAddr    = (uint32_t)(&u32SrcBuf[0]);
    stcDmaInit.u32SrcAddrInc = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;

    (void)DMA_Init(DMA_UNIT, DMA_CH, &stcDmaInit);

    (void)DMA_RepeatStructInit(&stcDmaRepeatInit);

    stcDmaRepeatInit.u32Mode     = DMA_RPT_SRC;
    stcDmaRepeatInit.u32SrcCount = DMA_SRC_RPT_SIZE;

    (void)DMA_RepeatInit(DMA_UNIT, DMA_CH, &stcDmaRepeatInit);
}

/**
 * @brief  DMA ch.3 transfer complete IRQ callback
 * @param  None
 * @retval None
 */
void DMA1_CH3_TransEnd_IrqCallback(void)
{
    m_u8DmaTcEnd = SET;
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
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);
    /* System clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();

    /* DMA/AOS FCG enable */
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

    while (RESET == m_u8DmaTcEnd) {
        if (SET == DMA_GetTransCompleteStatus(DMA_UNIT, DMA_FLAG_BTC_CH3)) {
            DMA_ClearTransCompleteStatus(DMA_UNIT, DMA_FLAG_BTC_CH3);
            AOS_SW_Trigger();
        }
    }
    if (0 != memcmp(u32DestBuf, u32ExpectDestBufData, sizeof(u32DestBuf))) {
        BSP_LED_On(LED_RED);    /* LED red */
    } else {
        BSP_LED_On(LED_BLUE);   /* LED blue, as expected */
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
