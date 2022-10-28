/**
 *******************************************************************************
 * @file  usart/usart_clocksync_int/source/main.c
 * @brief This example demonstrates clock sync data receive and transfer by interrupt.
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
 * @addtogroup USART_ClockSync_Interrupt
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
/**
 * @brief Buffer handle structure definition
 */
typedef struct {
    const uint8_t *pu8TxData;   /*!< Pointer to TX buffer */
    uint32_t      u32TxSize;    /*!< TX buffer size       */
    __IO uint32_t u32TxCount;   /*!< TX count             */
    uint8_t       *pu8RxData;   /*!< Pointer to RX buffer */
    uint32_t      u32RxSize;    /*!< RX buffer size */
    __IO uint32_t u32RxCount;   /*!< RX count       */
} stc_buf_handle_t;

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* USART CK/RX/TX pin definition */
#define USART_CK_PORT                   (GPIO_PORT_D)   /* PD7: USART2_CK */
#define USART_CK_PIN                    (GPIO_PIN_07)
#define USART_CK_GPIO_FUNC              (GPIO_FUNC_7)

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
static stc_buf_handle_t m_stcBufHandle;

static const uint8_t m_au8TxData[] = "USART clock-sync test.";
static uint8_t m_au8RxData[COM_DATA_LEN];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Enable send&&receive an amount of data(non-blocking).
 * @param  [in] USARTx                  pointer to a USART instance.
 * @param  [in] pstcBufHandle           pointer to a stc_buf_handle_t structure.
 * @param  [in] pu8TxData               Pointer to data transmitted buffer
 * @param  [out] pu8RxData              Pointer to data received buffer
 * @param  [in] u32Size                 Amount of data to be received
 * @retval int32_t:
 *           - LL_OK:                   Enable successfully.
 *           - LL_ERR_INVD_PARAM:       Invalid parameter
 */
static int32_t CLOCKSYNC_TransReceive_INT(CM_USART_TypeDef *USARTx,
        stc_buf_handle_t *pstcBufHandle,
        const uint8_t *pu8TxData,
        uint8_t *pu8RxData,
        uint32_t u32Size)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;
    const uint32_t u32Func = (USART_RX | USART_INT_RX | USART_TX);

    if ((USARTx != NULL) && (pstcBufHandle != NULL) && \
            (pu8TxData != NULL) && (pu8RxData != NULL) && (u32Size > 0UL)) {
        pstcBufHandle->pu8RxData = pu8RxData;
        pstcBufHandle->u32RxSize = u32Size;
        pstcBufHandle->u32RxCount = 0UL;
        pstcBufHandle->pu8TxData = pu8TxData;
        pstcBufHandle->u32TxSize = u32Size;
        pstcBufHandle->u32TxCount = 0UL;

        USART_FuncCmd(USARTx, (u32Func | USART_INT_TX_EMPTY | USART_INT_TX_CPLT), DISABLE);
        if (USART_CLK_SRC_EXTCLK == USART_GetClockSrc(USARTx)) {
            USART_FuncCmd(USARTx, (u32Func | USART_INT_TX_EMPTY), ENABLE);
        } else {
            USART_FuncCmd(USARTx, (u32Func | USART_INT_TX_CPLT), ENABLE);
        }
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Send receive an amount of data in full-duplex mode (non-blocking) in IRQ handler.
 * @param  [in] USARTx                  pointer to a USART instance.
 * @param  [in] pstcBufHandle           pointer to a stc_buf_handle_t structure.
 * @retval None
 */
static void CLOCKSYNC_TransReceiveCallback(CM_USART_TypeDef *USARTx, stc_buf_handle_t *pstcBufHandle)
{
    if (pstcBufHandle->u32RxCount != pstcBufHandle->u32RxSize) {
        if (USART_GetStatus(USARTx, USART_FLAG_RX_FULL) != RESET) {
            pstcBufHandle->pu8RxData[pstcBufHandle->u32RxCount] = (uint8_t)USART_ReadData(USARTx);
            pstcBufHandle->u32RxCount++;
        }
    }

    /* Check the latest data received */
    if (pstcBufHandle->u32RxCount == pstcBufHandle->u32RxSize) {
        /* Disable the USART RXNE && Error Interrupt */
        USART_FuncCmd(USARTx, USART_INT_RX, DISABLE);
    } else {
        if (pstcBufHandle->u32TxCount != pstcBufHandle->u32TxSize) {
            if (USART_GetStatus(USARTx, USART_FLAG_TX_EMPTY) == SET) {
                USART_WriteData(USARTx, (uint16_t)(pstcBufHandle->pu8TxData[pstcBufHandle->u32TxCount]));
                pstcBufHandle->u32TxCount++;

                /* Check the latest data transmitted */
                if (pstcBufHandle->u32TxCount == pstcBufHandle->u32TxSize) {
                    USART_FuncCmd(USARTx, (USART_INT_TX_EMPTY | USART_INT_TX_CPLT), DISABLE);
                }
            }
        }
    }
}

#if (MASTER_DEVICE_ENABLE == DDL_ON)
/**
 * @brief  USART TX Complete IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxComplete_IrqCallback(void)
{
    CLOCKSYNC_TransReceiveCallback(USART_UNIT, &m_stcBufHandle);
}
#else
/**
 * @brief  USART TX Empty IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxEmpty_IrqCallback(void)
{
    CLOCKSYNC_TransReceiveCallback(USART_UNIT, &m_stcBufHandle);
}
#endif

/**
 * @brief  USART RX IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxFull_IrqCallback(void)
{
    CLOCKSYNC_TransReceiveCallback(USART_UNIT, &m_stcBufHandle);
}

/**
 * @brief  USART RX Error IRQ callback.
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
 * @brief  Main function of clock-sync interrupt project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t u32TxXferCount;
    uint32_t u32RxXferCount;
    stc_irq_signin_config_t stcIrqConfig;
    stc_usart_clocksync_init_t stcClockSyncInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Initialize BSP key. */
    BSP_KEY_Init();

    /* Configure USART RX/TX pin. */
    GPIO_SetFunc(USART_CK_PORT, USART_CK_PIN, USART_CK_GPIO_FUNC);
    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Initialize CLKSYNC function. */
    (void)USART_ClockSync_StructInit(&stcClockSyncInit);
#if (MASTER_DEVICE_ENABLE == DDL_ON)
    stcClockSyncInit.u32ClockSrc = USART_CLK_SRC_INTERNCLK;
    stcClockSyncInit.u32ClockDiv = USART_CLK_DIV64;
    stcClockSyncInit.u32Baudrate = 9600UL;
#else
    stcClockSyncInit.u32ClockSrc = USART_CLK_SRC_EXTCLK;
#endif
    if (LL_OK != USART_ClockSync_Init(USART_UNIT, &stcClockSyncInit, NULL)) {
        BSP_LED_On(LED_RED);
        for (;;) {
        }
    }

    /* Register RX error IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = USART_RX_ERR_IRQn;
    stcIrqConfig.enIntSrc = USART_RX_ERR_INT_SRC;
    stcIrqConfig.pfnCallback = &USART_RxError_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqConfig, DDL_IRQ_PRIO_03);

    /* Register RX full IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = USART_RX_FULL_IRQn;
    stcIrqConfig.enIntSrc = USART_RX_FULL_INT_SRC;
    stcIrqConfig.pfnCallback = &USART_RxFull_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqConfig, DDL_IRQ_PRIO_00);

#if (MASTER_DEVICE_ENABLE == DDL_ON)
    /* Register TX complete IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = USART_TX_CPLT_IRQn;
    stcIrqConfig.enIntSrc = USART_TX_CPLT_INT_SRC;
    stcIrqConfig.pfnCallback = &USART_TxComplete_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqConfig, DDL_IRQ_PRIO_02);
#else
    /* Register TX empty IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = USART_TX_EMPTY_IRQn;
    stcIrqConfig.enIntSrc = USART_TX_EMPTY_INT_SRC;
    stcIrqConfig.pfnCallback = &USART_TxEmpty_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqConfig, DDL_IRQ_PRIO_01);
#endif

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Wait key press */
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_1)) {
    }

    /* Start the transmission process*/
    (void)CLOCKSYNC_TransReceive_INT(USART_UNIT, &m_stcBufHandle, m_au8TxData, m_au8RxData, COM_DATA_LEN);

    /* Wait tranmission complete */
    do {
        u32TxXferCount = m_stcBufHandle.u32TxCount;
        u32RxXferCount = m_stcBufHandle.u32RxCount;
    } while ((u32TxXferCount != COM_DATA_LEN) || (u32RxXferCount != COM_DATA_LEN));

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
