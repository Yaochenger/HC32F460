/**
 *******************************************************************************
 * @file  iap/iap_ymodem_boot/source/com.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
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
#include "com.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  COM De-Initialize.
 * @param  None
 * @retval None
 */
void COM_DeInit(void)
{
    USART_DeInit(MODEM_USART_UNIT);
    /* Disable USART clock */
    MODEM_USART_CLK_CTRL(DISABLE);
    /* Configure USART RX/TX pin */
    GPIO_SetFunc(MODEM_USART_RX_PORT, MODEM_USART_RX_PIN, GPIO_FUNC_0);
    GPIO_SetFunc(MODEM_USART_TX_PORT, MODEM_USART_TX_PIN, GPIO_FUNC_0);
}


/**
 * @brief  COM Initialize.
 * @param  None
 * @retval None
 */
void COM_Init(void)
{
    stc_usart_uart_init_t stcUartInit;

    /* Configure USART RX/TX pin */
    GPIO_SetFunc(MODEM_USART_RX_PORT, MODEM_USART_RX_PIN, MODEM_USART_RX_FUNC);
    GPIO_SetFunc(MODEM_USART_TX_PORT, MODEM_USART_TX_PIN, MODEM_USART_TX_FUNC);
    /* Enable USART Clock. */
    MODEM_USART_CLK_CTRL(ENABLE);
    /* Initialize UART */
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockDiv      = USART_CLK_DIV4;
    stcUartInit.u32Baudrate      = MODEM_USART_BAUD_RATE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;
    USART_UART_Init(MODEM_USART_UNIT, &stcUartInit, NULL);
    /* Enable RX/TX function */
    USART_FuncCmd(MODEM_USART_UNIT, (USART_RX | USART_TX), ENABLE);
}

/**
 * @brief  COM send data.
 * @param  [in] pu8Buff                 Pointer to the buffer to be sent
 * @param  [in] u16Len                  Send buffer length
 * @retval None
 */
void COM_SendData(uint8_t *pu8Buff, uint16_t u16Len)
{
    USART_UART_Trans(MODEM_USART_UNIT, pu8Buff, u16Len, USART_MAX_TIMEOUT);
}

/**
 * @brief  COM receive data.
 * @param  [out] pu8Buff                Pointer to the buffer to be sent
 * @param  [in]  u16Len                 Receive data length
 * @param  [in]  u32Timeout             Receive timeout(ms)
 * @retval int32_t:
 *           - LL_OK: Receive data finished
 *           - LL_ERR: Receive error
 *           - LL_ERR_INVD_PARAM: u32Len value is 0 or the pointer pvBuf value is NULL.
 */
int32_t COM_RecvData(uint8_t *pu8Buff, uint16_t u16Len, uint32_t u32Timeout)
{
    if ((NULL == pu8Buff) || (0U == u16Len)) {
        return LL_ERR_INVD_PARAM;
    }

    /* Clear the RX error */
    if (RESET != USART_GetStatus(MODEM_USART_UNIT, (USART_FLAG_OVERRUN | USART_FLAG_FRAME_ERR | USART_FLAG_PARITY_ERR))) {
        USART_ClearStatus(MODEM_USART_UNIT, (USART_FLAG_OVERRUN | USART_FLAG_FRAME_ERR | USART_FLAG_PARITY_ERR));
    }
    if (LL_OK != USART_UART_Receive(MODEM_USART_UNIT, pu8Buff, u16Len, u32Timeout * 1000U)) {
        return LL_ERR;
    }
    return LL_OK;
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
