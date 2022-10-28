/*******************************************************************************
* @file  dmac/dmac_channel_reconfig/source/main.c
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

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* DMAC */
#define DMA_UNIT                (CM_DMA2)
#define DMA_CH                  (DMA_CH0)

#define DMA_TC                  (20U)
#define DMA_BC                  (1U)
#define DMA_DW                  (DMA_DATAWIDTH_32BIT)

#define DMA_SRC_RPT_SIZE        (7U)

#define DMA_TRIGGER_CH          (AOS_DMA2_0)
#define DMA_RC_TRIGGER_SRC      (BSP_KEY_KEY10_EVT)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
int32_t CmpRet = 1;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static const uint32_t u32SrcBuf[22] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                       11, 12, 13, 14, 15, 16, 17, 18,
                                       19, 20, 21, 22
                                      };
static uint32_t u32DestBuf[22] = {0};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  External 01 callback
 * @param  None
 * @retval None
 */
void BSP_KEY_KEY10_IrqHandler(void)
{
    static uint32_t u32ExpectDestBufData[22] = {1, 2, 3, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0};

    EXTINT_ClearExtIntStatus(BSP_KEY_KEY10_EXTINT);
    AOS_SW_Trigger();

    while (SET != DMA_GetTransCompleteStatus(DMA_UNIT, (DMA_FLAG_TC_CH0 << DMA_CH))) {
        AOS_SW_Trigger();
    }
    CmpRet = memcmp(u32DestBuf, u32ExpectDestBufData, sizeof(u32DestBuf));
}

/**
 * @brief  DMA basic and channel re_config function init
 * @param  None
 * @retval None
 */
static void DmaInit(void)
{
    stc_dma_init_t stcDmaInit;
    stc_dma_repeat_init_t stcDmaRepeatInit;

    AOS_SetTriggerEventSrc(DMA_TRIGGER_CH, EVT_SRC_AOS_STRG);

    (void)DMA_StructInit(&stcDmaInit);

    stcDmaInit.u32IntEn      = DMA_INT_DISABLE;
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
 * @brief  DMA basic and channel re_config function init
 * @param  None
 * @retval None
 */
static void DmaReconfigInit(void)
{
    stc_dma_reconfig_init_t stcDmaReconfigInit;

    (void)DMA_ReconfigStructInit(&stcDmaReconfigInit);
    stcDmaReconfigInit.u32CountMode      = DMA_RC_CNT_SRC;
    stcDmaReconfigInit.u32SrcAddrMode    = DMA_RC_SRC_ADDR_RPT;
    stcDmaReconfigInit.u32DestAddrMode   = DMA_RC_DEST_ADDR_KEEP;

    (void)DMA_ReconfigInit(DMA_UNIT, DMA_CH, &stcDmaReconfigInit);

    AOS_SetTriggerEventSrc(AOS_DMA_RC, DMA_RC_TRIGGER_SRC);

    DMA_ReconfigCmd(DMA_UNIT, ENABLE);
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

    /* Clk initialization */
    BSP_CLK_Init();
    /* Led initialization */
    BSP_LED_Init();
    /* Key initialization */
    BSP_KEY_Init();

    /* Enable DMA & AOS clock. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DMA2 | FCG0_PERIPH_AOS, ENABLE);

    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Config DMA */
    DmaInit();

    /* Re-config DMA */
    DmaReconfigInit();

    /* Enable DMA. */
    DMA_Cmd(DMA_UNIT, ENABLE);
    /* Clear DMA transfer complete interrupt flag. */
    DMA_ClearTransCompleteStatus(DMA_UNIT, (DMA_FLAG_TC_CH0 << DMA_CH));
    /* Enable DMA channel. */
    (void)DMA_ChCmd(DMA_UNIT, DMA_CH, ENABLE);

    /* transfer 3 datas base initial config */
    AOS_SW_Trigger();
    AOS_SW_Trigger();
    AOS_SW_Trigger();

    /* KEY10 */
    while (PIN_RESET != GPIO_ReadInputPins(BSP_KEY_KEY10_PORT, BSP_KEY_KEY10_PIN)) {
        ;
    }

    if (0 == CmpRet) {
        BSP_LED_On(LED_BLUE);    /* Meet the expected */
    } else {
        BSP_LED_On(LED_RED);    /* Don't meet the expected */
    }

    for (;;) {
        ;
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
