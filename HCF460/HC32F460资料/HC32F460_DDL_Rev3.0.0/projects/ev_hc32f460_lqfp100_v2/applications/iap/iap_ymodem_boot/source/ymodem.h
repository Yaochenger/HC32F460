/**
 *******************************************************************************
 * @file  iap/iap_ymodem_boot/source/ymodem.h
 * @brief This file contains all the functions prototypes of the ymodem.
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
#ifndef __YMODEM_H__
#define __YMODEM_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_utility.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Communication status definition */
#define YMODEM_COM_OK                   (0x00U)
#define YMODEM_COM_ERR                  (0x01U)
#define YMODEM_COM_ABORT                (0x02U)
#define YMODEM_COM_LIMIT                (0x03U)
#define YMODEM_COM_FLASH_ERR            (0x04U)

/*
 ------------------------ Packet structure of YModem -----------------------------
 * | 0      | 1     | 2      | 3       | 4       | ... | n+4     | n+5   | n+6   |
 * |-----------------------------------------------------------------------------|
 * | unused | start | number | !number | data[0] | ... | data[n] | crc_H | crc_L |
 */
/* Packet structure defines */
#define PACKET_START_INDEX              (1U)
#define PACKET_NUM_INDEX                (2U)
#define PACKET_XORNUM_INDEX             (3U)
#define PACKET_DATA_INDEX               (4U)

#define PACKET_HEAD_SIZE                (3U)
#define PACKET_CRC_SIZE                 (2U)
#define PACKET_OVERHEAD_SIZE            (PACKET_HEAD_SIZE + PACKET_CRC_SIZE - 1U)
#define PACKET_SOH_SIZE                 (128U)
#define PACKET_STX_SIZE                 (1024U)

#define FILE_NAME_LEN                   (64U)
#define FILE_SIZE_LEN                   (16U)

/* YModem command defines */
#define YMODEM_SOH                      (0x01U) /* Start of 128-byte data packet  */
#define YMODEM_STX                      (0x02U) /* Start of 1024-byte data packet */
#define YMODEM_EOT                      (0x04U) /* End of transmission */
#define YMODEM_ACK                      (0x06U) /* Acknowledge */
#define YMODEM_NAK                      (0x15U) /* Negative acknowledge */
#define YMODEM_CAN                      (0x18U) /* Two of these in succession aborts transfer */
#define YMODEM_CRC16                    (0x43U) /* 'C' == 0x43, request 16-bit CRC */
#define YMODEM_NUM_XOR_BYTE             (0xFFU)

#define YMODEM_ABORT1                   (0x41U)  /* 'A' == 0x41, abort by user */
#define YMODEM_ABORT2                   (0x61U)  /* 'a' == 0x61, abort by user */

#define YMODEM_MAX_ERR                  (5U)
#define YMODEM_NAK_TIMEOUT              (50000UL)
#define YMODEM_RECV_TIMEOUT             (5000U) /* Five second retry delay */
#define YMODEM_RECV_WAITFOREVER         (0xFFFFFFFF)

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
void YModem_Int2Str(uint8_t *pu8Str, uint32_t u32Value);
int32_t YModem_Str2Int(uint32_t *pu32Value, uint8_t *pu8Str);
int32_t YModem_Receive(uint8_t *pu8FileName, uint32_t *pu32Size);
int32_t YModem_Transmit(uint8_t *pu8Buf, const uint8_t *pu8FileName, uint32_t fileSize);

#ifdef __cplusplus
}
#endif

#endif /* __MODEM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
