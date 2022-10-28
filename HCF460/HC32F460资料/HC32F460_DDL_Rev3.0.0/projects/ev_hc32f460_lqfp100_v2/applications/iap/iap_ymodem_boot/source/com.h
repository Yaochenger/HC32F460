/**
 *******************************************************************************
 * @file  iap/iap_ymodem_boot/source/com.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
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
#ifndef __COM_H__
#define __COM_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_fcg.h"
#include "hc32_ll_gpio.h"
#include "hc32_ll_usart.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define MODEM_USART_UNIT                (CM_USART4)
#define MODEM_USART_CLK_CTRL(state)     FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART4, state)
#define MODEM_USART_BAUD_RATE           (115200UL)

#define MODEM_USART_RX_PORT             (GPIO_PORT_B)
#define MODEM_USART_RX_PIN              (GPIO_PIN_09)
#define MODEM_USART_RX_FUNC             (GPIO_FUNC_37)

#define MODEM_USART_TX_PORT             (GPIO_PORT_E)
#define MODEM_USART_TX_PIN              (GPIO_PIN_06)
#define MODEM_USART_TX_FUNC             (GPIO_FUNC_36)

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
void COM_DeInit(void);
void COM_Init(void);
void COM_SendData(uint8_t *pu8Buff, uint16_t u16Len);
int32_t COM_RecvData(uint8_t *pu8Buff, uint16_t u16Len, uint32_t u32Timeout);

#ifdef __cplusplus
}
#endif

#endif /* __COM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
