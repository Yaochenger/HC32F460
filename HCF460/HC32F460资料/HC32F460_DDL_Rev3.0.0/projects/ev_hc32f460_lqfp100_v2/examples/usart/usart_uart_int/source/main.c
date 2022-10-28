/**
 *******************************************************************************
 * @file  usart/usart_uart_int/source/main.c
 * @brief This example demonstrates UART data receive and transfer by interrupt.
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
 * @addtogroup USART_UART_Interrupt
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

/* USART interrupt definition */
#define USART_RX_ERR_IRQn               (INT000_IRQn)
#define USART_RX_ERR_INT_SRC            (INT_SRC_USART4_EI)

#define USART_RX_FULL_IRQn              (INT001_IRQn)
#define USART_RX_FULL_INT_SRC           (INT_SRC_USART4_RI)

#define USART_TX_EMPTY_IRQn             (INT002_IRQn)
#define USART_TX_EMPTY_INT_SRC          (INT_SRC_USART4_TI)

#define USART_TX_CPLT_IRQn              (INT003_IRQn)
#define USART_TX_CPLT_INT_SRC           (INT_SRC_USART4_TCI)

/* Ring buffer size */
#define RING_BUF_SIZE                   (500UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t m_au8DataBuf[RING_BUF_SIZE];
static stc_ring_buf_t m_stcRingBuf;
static __IO en_flag_status_t m_enTxCompleteFlag = SET;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  USART transmit data register empty IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxEmpty_IrqCallback(void)
{
    uint8_t u8Data;

    if (!BUF_Empty(&m_stcRingBuf)) {
        (void)BUF_Read(&m_stcRingBuf, &u8Data, 1UL);
        USART_WriteData(USART_UNIT, (uint16_t)u8Data);
    } else {
        USART_FuncCmd(USART_UNIT, USART_INT_TX_CPLT, ENABLE);
    }
}

/**
 * @brief  USART transmit complete IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxComplete_IrqCallback(void)
{
    m_enTxCompleteFlag = SET;
    USART_FuncCmd(USART_UNIT, (USART_TX | USART_INT_TX_CPLT | USART_INT_TX_EMPTY), DISABLE);
}

/**
 * @brief  USART RX IRQ callback
 * @param  None
 * @retval None
 */
static void USART_RxFull_IrqCallback(void)
{
    uint8_t u8Data = (uint8_t)USART_ReadData(USART_UNIT);

    (void)BUF_Write(&m_stcRingBuf, &u8Data, 1UL);
}

/**
 * @brief  USART error IRQ callback.
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
 * @brief  Instal IRQ handler.
 * @param  [in] pstcConfig      Pointer to struct @ref stc_irq_signin_config_t
 * @param  [in] u32Priority     Interrupt priority
 * @retval None
 */
static void INTC_IrqInstalHandler(const stc_irq_signin_config_t *pstcConfig, uint32_t u32Priority)
{
    if (NULL != pstcConfig) {
        (void)INTC_IrqSignIn(pstcConfig);
        NVIC_ClearPendingIRQ(pstcConfig->enIRQn);
        NVIC_SetPriority(pstcConfig->enIRQn, u32Priority);
        NVIC_EnableIRQ(pstcConfig->enIRQn);
    }
}

/**
 * @brief  Main function of UART interrupt project
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

    /* Configure USART RX/TX pin. */
    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Initialize ring buffer function. */
    (void)BUF_Init(&m_stcRingBuf, m_au8DataBuf, sizeof(m_au8DataBuf));

    /* Initialize UART. */
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockDiv = USART_CLK_DIV64;
    stcUartInit.u32Baudrate = 115200UL;
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
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* Register RX full IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_RX_FULL_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_FULL_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxFull_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* Register TX empty IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_TX_EMPTY_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_TX_EMPTY_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_TxEmpty_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* Register TX complete IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_TX_CPLT_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_TX_CPLT_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_TxComplete_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Enable RX function */
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX), ENABLE);

    for (;;) {
        if ((SET == m_enTxCompleteFlag) && !BUF_Empty(&m_stcRingBuf)) {
            m_enTxCompleteFlag = RESET;
            USART_FuncCmd(USART_UNIT, (USART_TX | USART_INT_TX_EMPTY), ENABLE);
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
