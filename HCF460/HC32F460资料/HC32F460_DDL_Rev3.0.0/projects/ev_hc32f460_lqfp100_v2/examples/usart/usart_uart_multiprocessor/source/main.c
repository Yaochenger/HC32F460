/**
 *******************************************************************************
 * @file  usart/usart_uart_multiprocessor/source/main.c
 * @brief This example demonstrates UART multi-processor receive and transfer.
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
 * @addtogroup USART_UART_MultiProcessor
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
#define USART_RX_PORT                   (GPIO_PORT_A)   /* PA3: USART2_RX */
#define USART_RX_PIN                    (GPIO_PIN_03)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_37)

#define USART_TX_PORT                   (GPIO_PORT_A)   /* PA2: USART2_TX */
#define USART_TX_PIN                    (GPIO_PIN_02)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_36)

/* USART unit definition */
#define USART_UNIT                      (CM_USART2)
#define USART_FCG_ENABLE()              (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART2, ENABLE))

/* USART interrupt definition */
#define USART_RX_ERR_IRQn               (INT000_IRQn)
#define USART_RX_ERR_INT_SRC            (INT_SRC_USART2_EI)

#define USART_RX_FULL_IRQn              (INT001_IRQn)
#define USART_RX_FULL_INT_SRC           (INT_SRC_USART2_RI)

#define USART_TX_EMPTY_IRQn             (INT002_IRQn)
#define USART_TX_EMPTY_INT_SRC          (INT_SRC_USART2_TI)

#define USART_TX_CPLT_IRQn              (INT003_IRQn)
#define USART_TX_CPLT_INT_SRC           (INT_SRC_USART2_TCI)

/* USART multiple processor ID definition */
#define USART_MASTER_STATION_ID         (0x20U)
#define USART_SLAVE_STATION_ID          (0x21U)

/* Communication data size */
#define COM_DATA_LEN                    (ARRAY_SZ(m_au8TxData))

/* DDL_ON: master device / DDL_OFF: slave device */
#define MASTER_DEVICE_ENABLE            (DDL_ON)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static en_functional_state_t m_enTxIdState = ENABLE;
static en_functional_state_t m_enUartSilenceState;

static uint32_t m_u32TxIndex;
static const uint8_t m_au8TxData[] = "USART multiple-processor test.";
static __IO en_flag_status_t m_enTxCompleteFlag = RESET;

static uint32_t m_u32RxIndex;
static uint8_t m_au8RxData[COM_DATA_LEN];
static __IO en_flag_status_t m_enRxCompleteFlag = RESET;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Set silence state.
 * @param  [in] enNewState              An @ref en_functional_state_t enumeration value.
 * @retval None
 */
static void USART_SetSilenceState(en_functional_state_t enNewState)
{
    m_enUartSilenceState = enNewState;
}

/**
 * @brief  Get silence state.
 * @param  [in] None
 * @retval Returned value can be one of the following values:
 *           - ENABLE:               The silence state is enabled.
 *           - DISABLE:              The silence state is disabled.
 */
static en_functional_state_t USART_GetSilenceState(void)
{
    return m_enUartSilenceState;
}

/**
 * @brief  UART TX Empty IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxEmpty_IrqCallback(void)
{
    uint16_t u16Id;
    static const uint8_t *pu8TxData;

    if (ENABLE == m_enTxIdState) {
#if (MASTER_DEVICE_ENABLE == DDL_ON)
        u16Id = USART_SLAVE_STATION_ID;
        pu8TxData = m_au8TxData;
#else
        u16Id = USART_MASTER_STATION_ID;
        pu8TxData = m_au8RxData;
#endif
        USART_WriteID(USART_UNIT, u16Id);
        m_enTxIdState = DISABLE;
    } else {
        if (m_u32TxIndex < COM_DATA_LEN) {
            USART_WriteData(USART_UNIT, pu8TxData[m_u32TxIndex]);
            m_u32TxIndex += 1UL;
        } else {
            USART_FuncCmd(USART_UNIT, USART_INT_TX_CPLT, ENABLE);
        }
    }
}

/**
 * @brief  UART TX Complete IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxComplete_IrqCallback(void)
{
    m_enTxCompleteFlag = SET;
    USART_FuncCmd(USART_UNIT, (USART_TX | USART_INT_TX_EMPTY | USART_INT_TX_CPLT), DISABLE);
}

/**
 * @brief  UART RX full IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxFull_IrqCallback(void)
{
    uint8_t u8RxData;

    u8RxData = (uint8_t)USART_ReadData(USART_UNIT);

    if ((RESET == USART_GetStatus(USART_UNIT, USART_FLAG_MX_PROCESSOR)) &&
            (DISABLE == USART_GetSilenceState())) {
        m_au8RxData[m_u32RxIndex] = u8RxData;
        m_u32RxIndex += 1UL;

        if (m_u32RxIndex == COM_DATA_LEN) {
            m_enRxCompleteFlag = SET;
            /* Disable RX & RX no empty interrupt function */
            USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX), DISABLE);
        }
    } else {
#if (MASTER_DEVICE_ENABLE == DDL_ON)
        if (USART_MASTER_STATION_ID != u8RxData) {
#else
        if (USART_SLAVE_STATION_ID != u8RxData) {
#endif
            USART_SilenceCmd(USART_UNIT, ENABLE);
            USART_SetSilenceState(ENABLE);
        } else {
            USART_SetSilenceState(DISABLE);
        }
    }
}

/**
 * @brief  UART RX Error IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxErr_IrqCallback(void)
{
    if (SET == USART_GetStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR))) {
        (void)USART_ReadData(USART_UNIT);
    }

    /* Clear flag */
    USART_ClearStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR | USART_FLAG_OVERRUN));
}

/**
 * @brief  Instal IRQ handler.
 * @param  [in] pstcConfig              Pointer to struct @ref stc_irq_signin_config_t
 * @param  [in] u32Priority             Interrupt priority
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
 * @brief  Main function of UART multiple processor project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_irq_signin_config_t stcIrqSigninConfig;
    stc_usart_multiprocessor_init_t stcUartMultiProcessorInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Initialize BSP key. */
    BSP_KEY_Init();

    /* Configure USART RX/TX pin. */
    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Initialize multi-processor. */
    (void)USART_MultiProcessor_StructInit(&stcUartMultiProcessorInit);
    stcUartMultiProcessorInit.u32ClockDiv = USART_CLK_DIV64;
    stcUartMultiProcessorInit.u32Baudrate = 9600UL;
    stcUartMultiProcessorInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;
    if (LL_OK != USART_MultiProcessor_Init(USART_UNIT, &stcUartMultiProcessorInit, NULL)) {
        BSP_LED_On(LED_RED);
        for (;;) {
        }
    }

    /* Register RX error IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_RX_ERR_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_ERR_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxErr_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* Register RX full IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_RX_FULL_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_FULL_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxFull_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_00);

    /* Register TX IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_TX_EMPTY_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_TX_EMPTY_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_TxEmpty_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* Register TC IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_TX_CPLT_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_TX_CPLT_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_TxComplete_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Enable silence mode */
    USART_SilenceCmd(USART_UNIT, ENABLE);
    USART_SetSilenceState(ENABLE);

#if (MASTER_DEVICE_ENABLE == DDL_ON)
    /* Wait key press */
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_1)) {
    }

    /* Master send data */
    USART_FuncCmd(USART_UNIT, (USART_TX | USART_INT_TX_EMPTY), ENABLE);
    while (RESET == m_enTxCompleteFlag) {
    }

    /* Enable silence mode */
    USART_SilenceCmd(USART_UNIT, ENABLE);
    USART_SetSilenceState(ENABLE);

    /* Master receive data */
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX), ENABLE);
    while (RESET == m_enRxCompleteFlag) {
    }
#else
    /* Slave receive data */
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX), ENABLE);
    while (RESET == m_enRxCompleteFlag) {
    }

    /* Enable silence mode */
    USART_SilenceCmd(USART_UNIT, ENABLE);
    USART_SetSilenceState(ENABLE);

    /* Slave send data */
    USART_FuncCmd(USART_UNIT, (USART_TX | USART_INT_TX_EMPTY), ENABLE);
    while (RESET == m_enTxCompleteFlag) {
    }
#endif

    /* Compare data */
    if (0 == memcmp(m_au8RxData, m_au8TxData, COM_DATA_LEN)) {
        BSP_LED_On(LED_BLUE);
    } else {
        BSP_LED_On(LED_RED);
    }

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
