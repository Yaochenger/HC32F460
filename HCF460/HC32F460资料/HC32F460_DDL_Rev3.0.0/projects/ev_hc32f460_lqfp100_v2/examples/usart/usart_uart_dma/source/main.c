/**
 *******************************************************************************
 * @file  usart/usart_uart_dma/source/main.c
 * @brief This example demonstrates UART data receive and transfer by DMA.
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
 * @addtogroup USART_UART_DMA
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* DMA definition */
#define RX_DMA_UNIT                     (CM_DMA1)
#define RX_DMA_CH                       (DMA_CH0)
#define RX_DMA_FCG_ENABLE()             (FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DMA1, ENABLE))
#define RX_DMA_TRIG_SEL                 (AOS_DMA1_0)
#define RX_DMA_TRIG_EVT_SRC             (EVT_SRC_USART4_RI)
#define RX_DMA_RECONF_TRIG_SEL          (AOS_DMA_RC)
#define RX_DMA_RECONF_TRIG_EVT_SRC      (EVT_SRC_AOS_STRG)
#define RX_DMA_TC_INT                   (DMA_INT_TC_CH0)
#define RX_DMA_TC_FLAG                  (DMA_FLAG_TC_CH0)
#define RX_DMA_TC_IRQn                  (INT000_IRQn)
#define RX_DMA_TC_INT_SRC               (INT_SRC_DMA1_TC0)

#define TX_DMA_UNIT                     (CM_DMA2)
#define TX_DMA_CH                       (DMA_CH0)
#define TX_DMA_FCG_ENABLE()             (FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DMA2, ENABLE))
#define TX_DMA_TRIG_SEL                 (AOS_DMA2_0)
#define TX_DMA_TRIG_EVT_SRC             (EVT_SRC_USART4_TI)
#define TX_DMA_TC_INT                   (DMA_INT_TC_CH0)
#define TX_DMA_TC_FLAG                  (DMA_FLAG_TC_CH0)
#define TX_DMA_TC_IRQn                  (INT001_IRQn)
#define TX_DMA_TC_INT_SRC               (INT_SRC_DMA2_TC0)

/* Timer0 unit & channel definition */
#define TMR0_UNIT                       (CM_TMR0_2)
#define TMR0_CH                         (TMR0_CH_B)
#define TMR0_FCG_ENABLE()               (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR0_2, ENABLE))

/* USART RX/TX pin definition */
#define USART_RX_PORT                   (GPIO_PORT_B)   /* PB9: USART4_RX */
#define USART_RX_PIN                    (GPIO_PIN_09)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_37)

#define USART_TX_PORT                   (GPIO_PORT_E)   /* PE6: USART4_TX */
#define USART_TX_PIN                    (GPIO_PIN_06)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_36)

/* USART unit definition */
#define USART_UNIT                      (CM_USART4)
#define USART_FCG_ENABLE()              (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART4, ENABLE))

/* USART baudrate definition */
#define USART_BAUDRATE                  (115200UL)

/* USART timeout bits definition */
#define USART_TIMEOUT_BITS              (2000U)

/* USART interrupt definition */
#define USART_TX_CPLT_IRQn              (INT002_IRQn)
#define USART_TX_CPLT_INT_SRC           (INT_SRC_USART4_TCI)

#define USART_RX_ERR_IRQn               (INT003_IRQn)
#define USART_RX_ERR_INT_SRC            (INT_SRC_USART4_EI)

#define USART_RX_TIMEOUT_IRQn           (INT004_IRQn)
#define USART_RX_TIMEOUT_INT_SRC        (INT_SRC_USART4_RTO)

/* Application frame length max definition */
#define APP_FRAME_LEN_MAX               (500U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static __IO en_flag_status_t m_enRxFrameEnd;
static __IO uint16_t m_u16RxLen;
static uint8_t m_au8RxBuf[APP_FRAME_LEN_MAX];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  DMA transfer complete IRQ callback function.
 * @param  None
 * @retval None
 */
static void RX_DMA_TC_IrqCallback(void)
{
    m_enRxFrameEnd = SET;
    m_u16RxLen = APP_FRAME_LEN_MAX;

    USART_FuncCmd(USART_UNIT, USART_RX_TIMEOUT, DISABLE);

    DMA_ClearTransCompleteStatus(RX_DMA_UNIT, RX_DMA_TC_FLAG);
}

/**
 * @brief  DMA transfer complete IRQ callback function.
 * @param  None
 * @retval None
 */
static void TX_DMA_TC_IrqCallback(void)
{
    USART_FuncCmd(USART_UNIT, USART_INT_TX_CPLT, ENABLE);

    DMA_ClearTransCompleteStatus(TX_DMA_UNIT, TX_DMA_TC_FLAG);
}

/**
 * @brief  Initialize DMA.
 * @param  None
 * @retval int32_t:
 *           - LL_OK:                   Initialize successfully.
 *           - LL_ERR_INVD_PARAM:       Initialization paramters is invalid.
 */
static int32_t DMA_Config(void)
{
    int32_t i32Ret;
    stc_dma_init_t stcDmaInit;
    stc_dma_llp_init_t stcDmaLlpInit;
    stc_irq_signin_config_t stcIrqSignConfig;
    static stc_dma_llp_descriptor_t stcLlpDesc;

    /* DMA&AOS FCG enable */
    RX_DMA_FCG_ENABLE();
    TX_DMA_FCG_ENABLE();
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

    /* USART_RX_DMA */
    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32IntEn = DMA_INT_ENABLE;
    stcDmaInit.u32BlockSize = 1UL;
    stcDmaInit.u32TransCount = ARRAY_SZ(m_au8RxBuf);
    stcDmaInit.u32DataWidth = DMA_DATAWIDTH_8BIT;
    stcDmaInit.u32DestAddr = (uint32_t)m_au8RxBuf;
    stcDmaInit.u32SrcAddr = ((uint32_t)(&USART_UNIT->DR) + 2UL);
    stcDmaInit.u32SrcAddrInc = DMA_SRC_ADDR_FIX;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;
    i32Ret = DMA_Init(RX_DMA_UNIT, RX_DMA_CH, &stcDmaInit);
    if (LL_OK == i32Ret) {
        (void)DMA_LlpStructInit(&stcDmaLlpInit);
        stcDmaLlpInit.u32State = DMA_LLP_ENABLE;
        stcDmaLlpInit.u32Mode  = DMA_LLP_WAIT;
        stcDmaLlpInit.u32Addr  = (uint32_t)&stcLlpDesc;
        (void)DMA_LlpInit(RX_DMA_UNIT, RX_DMA_CH, &stcDmaLlpInit);

        stcLlpDesc.SARx   = stcDmaInit.u32SrcAddr;
        stcLlpDesc.DARx   = stcDmaInit.u32DestAddr;
        stcLlpDesc.DTCTLx = (stcDmaInit.u32TransCount << DMA_DTCTL_CNT_POS) | (stcDmaInit.u32BlockSize << DMA_DTCTL_BLKSIZE_POS);;
        stcLlpDesc.LLPx   = (uint32_t)&stcLlpDesc;
        stcLlpDesc.CHCTLx = stcDmaInit.u32SrcAddrInc | stcDmaInit.u32DestAddrInc | stcDmaInit.u32DataWidth |  \
                            stcDmaInit.u32IntEn      | stcDmaLlpInit.u32State    | stcDmaLlpInit.u32Mode;

        DMA_ReconfigLlpCmd(RX_DMA_UNIT, RX_DMA_CH, ENABLE);
        DMA_ReconfigCmd(RX_DMA_UNIT, ENABLE);
        AOS_SetTriggerEventSrc(RX_DMA_RECONF_TRIG_SEL, RX_DMA_RECONF_TRIG_EVT_SRC);

        stcIrqSignConfig.enIntSrc = RX_DMA_TC_INT_SRC;
        stcIrqSignConfig.enIRQn  = RX_DMA_TC_IRQn;
        stcIrqSignConfig.pfnCallback = &RX_DMA_TC_IrqCallback;
        (void)INTC_IrqSignIn(&stcIrqSignConfig);
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        AOS_SetTriggerEventSrc(RX_DMA_TRIG_SEL, RX_DMA_TRIG_EVT_SRC);

        DMA_Cmd(RX_DMA_UNIT, ENABLE);
        DMA_TransCompleteIntCmd(RX_DMA_UNIT, RX_DMA_TC_INT, ENABLE);
        (void)DMA_ChCmd(RX_DMA_UNIT, RX_DMA_CH, ENABLE);
    }

    /* USART_TX_DMA */
    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32IntEn = DMA_INT_ENABLE;
    stcDmaInit.u32BlockSize = 1UL;
    stcDmaInit.u32TransCount = ARRAY_SZ(m_au8RxBuf);
    stcDmaInit.u32DataWidth = DMA_DATAWIDTH_8BIT;
    stcDmaInit.u32DestAddr = (uint32_t)(&USART_UNIT->DR);
    stcDmaInit.u32SrcAddr = (uint32_t)m_au8RxBuf;
    stcDmaInit.u32SrcAddrInc = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_FIX;
    i32Ret = DMA_Init(TX_DMA_UNIT, TX_DMA_CH, &stcDmaInit);
    if (LL_OK == i32Ret) {
        stcIrqSignConfig.enIntSrc = TX_DMA_TC_INT_SRC;
        stcIrqSignConfig.enIRQn  = TX_DMA_TC_IRQn;
        stcIrqSignConfig.pfnCallback = &TX_DMA_TC_IrqCallback;
        (void)INTC_IrqSignIn(&stcIrqSignConfig);
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        AOS_SetTriggerEventSrc(TX_DMA_TRIG_SEL, TX_DMA_TRIG_EVT_SRC);

        DMA_Cmd(TX_DMA_UNIT, ENABLE);
        DMA_TransCompleteIntCmd(TX_DMA_UNIT, TX_DMA_TC_INT, ENABLE);
    }

    return i32Ret;
}

/**
 * @brief  Configure TMR0.
 * @param  [in] u16TimeoutBits:         Timeout bits
 * @retval None
 */
static void TMR0_Config(uint16_t u16TimeoutBits)
{
    uint16_t u16CompareValue;
    stc_tmr0_init_t stcTmr0Init;

    TMR0_FCG_ENABLE();

    /* Initialize TMR0 base function. */
    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC_XTAL32;
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV8;
    stcTmr0Init.u32Func     = TMR0_FUNC_CMP;
    if (TMR0_CLK_DIV1 == stcTmr0Init.u32ClockDiv) {
        u16CompareValue = (u16TimeoutBits - 4U);
    } else if (TMR0_CLK_DIV2 == stcTmr0Init.u32ClockDiv) {
        u16CompareValue = (u16TimeoutBits / 2U - 2U);
    } else {
        u16CompareValue = (u16TimeoutBits / ((uint16_t)1U << (stcTmr0Init.u32ClockDiv >> TMR0_BCONR_CKDIVA_POS)) - 1U);
    }
    stcTmr0Init.u16CompareValue = u16CompareValue;
    (void)TMR0_Init(TMR0_UNIT, TMR0_CH, &stcTmr0Init);

    TMR0_HWStartCondCmd(TMR0_UNIT, TMR0_CH, ENABLE);
    TMR0_HWClearCondCmd(TMR0_UNIT, TMR0_CH, ENABLE);
}

/**
 * @brief  USART RX timeout IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxTimeout_IrqCallback(void)
{
    if (m_enRxFrameEnd != SET) {
        m_enRxFrameEnd = SET;
        m_u16RxLen = APP_FRAME_LEN_MAX - (uint16_t)DMA_GetTransCount(RX_DMA_UNIT, RX_DMA_CH);

        /* Trigger for re-config USART RX DMA */
        AOS_SW_Trigger();
    }

    TMR0_Stop(TMR0_UNIT, TMR0_CH);

    USART_ClearStatus(USART_UNIT, USART_FLAG_RX_TIMEOUT);
}

/**
 * @brief  USART TX complete IRQ callback function.
 * @param  None
 * @retval None
 */
static void USART_TxComplete_IrqCallback(void)
{
    USART_FuncCmd(USART_UNIT, (USART_TX | USART_INT_TX_CPLT), DISABLE);

    TMR0_Stop(TMR0_UNIT, TMR0_CH);

    USART_ClearStatus(USART_UNIT, USART_FLAG_RX_TIMEOUT);

    USART_FuncCmd(USART_UNIT, USART_RX_TIMEOUT, ENABLE);

    USART_ClearStatus(USART_UNIT, USART_FLAG_TX_CPLT);
}

/**
 * @brief  USART RX error IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxError_IrqCallback(void)
{
    if (SET == USART_GetStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR))) {
        (void)USART_ReadData(USART_UNIT);
    }

    USART_ClearStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR | USART_FLAG_OVERRUN));
}

/**
 * @brief  Main function of UART DMA project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_usart_uart_init_t stcUartInit;
    stc_irq_signin_config_t stcIrqSigninConfig;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Initialize DMA. */
    (void)DMA_Config();

    /* Initialize TMR0. */
    TMR0_Config(USART_TIMEOUT_BITS);

    /* Configure USART RX/TX pin. */
    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Initialize UART. */
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockDiv = USART_CLK_DIV64;
    stcUartInit.u32CKOutput = USART_CK_OUTPUT_ENABLE;
    stcUartInit.u32Baudrate = USART_BAUDRATE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;
    if (LL_OK != USART_UART_Init(USART_UNIT, &stcUartInit, NULL)) {
        BSP_LED_On(LED_RED);
        for (;;) {
        }
    }

    /* Register RX error IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_RX_ERR_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_ERR_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxError_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    /* Register TX complete IRQ handler. */
    stcIrqSigninConfig.enIRQn = USART_TX_CPLT_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_TX_CPLT_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_TxComplete_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    /* Register RX error IRQ handler. */
    stcIrqSigninConfig.enIRQn = USART_RX_ERR_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_ERR_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxError_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    /* Register RX timeout IRQ handler. */
    stcIrqSigninConfig.enIRQn = USART_RX_TIMEOUT_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_TIMEOUT_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxTimeout_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Enable TX && RX && RX interrupt function */
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX | USART_RX_TIMEOUT | \
                               USART_INT_RX_TIMEOUT), ENABLE);

    for (;;) {
        if (SET == m_enRxFrameEnd) {
            DMA_SetSrcAddr(TX_DMA_UNIT, TX_DMA_CH, (uint32_t)m_au8RxBuf);

            DMA_SetTransCount(TX_DMA_UNIT, TX_DMA_CH, m_u16RxLen);

            (void)DMA_ChCmd(TX_DMA_UNIT, TX_DMA_CH, ENABLE);

            USART_FuncCmd(USART_UNIT, USART_TX, ENABLE);

            m_enRxFrameEnd = RESET;
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
