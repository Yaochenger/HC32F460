/**
 *******************************************************************************
 * @file  iap/iap_app/source/modem.h
 * @brief This file contains all the functions prototypes of the Modem.
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
#ifndef __MODEM_H__
#define __MODEM_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_crc.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 * @brief Modem communication structure definition
 */
typedef struct {
    void (*SendData)(uint8_t *pu8Buff, uint16_t u16Len);
    int32_t (*RecvData)(uint8_t *pu8Buff, uint16_t u16Len, uint32_t u32Timeout);
} stc_modem_com_t;

/**
 * @brief Modem flash structure definition
 */
typedef struct {
    uint32_t u32FlashBase;
    uint32_t u32FlashSize;
    uint32_t u32SectorSize;
    int32_t (*CheckAddrAlign)(uint32_t u32Addr);
    int32_t (*EraseSector)(uint32_t u32Addr, uint32_t u32Size);
    int32_t (*WriteData)(uint32_t u32Addr, uint8_t *pu8Buff, uint32_t u32Len);
    int32_t (*ReadData)(uint32_t u32Addr, uint8_t *pu8Buff, uint32_t u32Len);
} stc_modem_flash_t;

/**
 * @brief Packet status definition
 */
#define PACKET_ACK_OK                   (0x00U)
#define PACKET_ACK_ERR                  (0x01U)
#define PACKET_ACK_ABORT                (0x02U)
#define PACKET_ACK_TIMEOUT              (0x03U)
#define PACKET_ACK_ADDR_ERR             (0x04U)

/**
 * @brief Packet command definition
 */
#define PACKET_CMD_HANDSHAKE            (0x20U)
#define PACKET_CMD_JUMP_TO_APP          (0x21U)
#define PACKET_CMD_APP_DOWNLOAD         (0x22U)
#define PACKET_CMD_APP_UPLOAD           (0x23U)
#define PACKET_CMD_ERASE_FLASH          (0x24U)
#define PACKET_CMD_FLASH_CRC            (0x25U)
#define PACKET_CMD_APP_UPGRADE          (0x26U)

/**
 * @brief Packet type definition
 */
#define PACKET_TYPE_CONTROL             (0x11U)
#define PACKET_TYPE_DATA                (0x12U)

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Frame structure definition */
#define FRAME_HEAD                      (0xAC6DU)
#define FRAME_SHELL_SIZE                (0x08U)
#define FRAME_NUM_XOR_BYTE              (0xFFU)

#define FRAME_HEAD_INDEX                (0x00U)
#define FRAME_NUM_INDEX                 (0x02U)
#define FRAME_XORNUM_INDEX              (0x03U)
#define FRAME_LENGTH_INDEX              (0x04U)
#define FRAME_PACKET_INDEX              (0x06U)

/* Packet structure definition */
#define PACKET_INSTRUCT_SIZE            (10U)
#define PACKET_DATA_SIZE                (512U)
#define PACKET_MIN_SIZE                 (PACKET_INSTRUCT_SIZE)
#define PACKET_MAX_SIZE                 (PACKET_DATA_SIZE + PACKET_INSTRUCT_SIZE)

#define PACKET_CMD_INDEX                (FRAME_PACKET_INDEX + 0x00U)
#define PACKET_TYPE_INDEX               (FRAME_PACKET_INDEX + 0x01U)
#define PACKET_RESULT_INDEX             (FRAME_PACKET_INDEX + 0x01U)
#define PACKET_ADDR_INDEX               (FRAME_PACKET_INDEX + 0x02U)
#define PACKET_DATA_INDEX               (FRAME_PACKET_INDEX + PACKET_INSTRUCT_SIZE)

/* Frame communication timeout(ms) definition */
#define FRAME_RECV_TIMEOUT              (5UL)
/* CRC16 init value */
#define FRAME_CRC16_INIT_VALUE          (0xA28CU)

/* APP configuration */
#define APP_EXIST_FLAG                  (0x67890123UL)
#define APP_UPGRADE_FLAG                (0xA5B6C7D8UL)

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
int32_t Modem_Process(stc_modem_com_t *pModemCom, stc_modem_flash_t *pModemFlash, uint32_t u32Timeout);

#ifdef __cplusplus
}
#endif

#endif /* __MODEM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
