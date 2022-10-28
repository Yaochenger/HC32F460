/**
 *******************************************************************************
 * @file  iap/iap_app/source/modem.c
 * @brief This file provides firmware functions to manage the Modem.
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
#include "modem.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static uint8_t u8FrameData[FRAME_SHELL_SIZE + PACKET_MAX_SIZE];

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
 * @brief  Modem receive frame.
 * @param  [in] pModemCom               Pointer to a @ref stc_modem_com_t structure
 * @param  [out] u8RxBuff               Pointer to the buffer which the data to be received
 * @param  [out] u16RxLen               Packet size
 * @param  [in]  u32Timeout             Receive timeout(ms)
 * @retval int32_t:
 *           - LL_OK: Receive finished
 *           - LL_ERR: Receive error
 */
static int32_t Modem_RecvFrame(stc_modem_com_t *pModemCom, uint8_t *u8RxBuff, uint16_t *u16RxLen, uint32_t u32Timeout)
{
    uint16_t u16Head, u16Crc16;
    uint16_t u16PacketSize;

    if (LL_OK == pModemCom->RecvData(&u8RxBuff[FRAME_HEAD_INDEX], 2U, u32Timeout)) {
        u16Head = u8RxBuff[FRAME_HEAD_INDEX] + ((uint16_t)u8RxBuff[FRAME_HEAD_INDEX + 1U] << 8U);
        if ((FRAME_HEAD == u16Head) && (LL_OK == pModemCom->RecvData(&u8RxBuff[FRAME_NUM_INDEX], 2U, u32Timeout))) {
            if ((u8RxBuff[FRAME_NUM_INDEX] == (u8RxBuff[FRAME_XORNUM_INDEX] ^ FRAME_NUM_XOR_BYTE)) &&
                    (LL_OK == pModemCom->RecvData(&u8RxBuff[FRAME_LENGTH_INDEX], 2U, u32Timeout))) {
                u16PacketSize = u8RxBuff[FRAME_LENGTH_INDEX] + ((uint16_t)u8RxBuff[FRAME_LENGTH_INDEX + 1U] << 8U);
                if (((u16PacketSize >= PACKET_MIN_SIZE) && (u16PacketSize <= PACKET_MAX_SIZE)) &&
                        (LL_OK == pModemCom->RecvData(&u8RxBuff[FRAME_PACKET_INDEX], u16PacketSize + 2U, u32Timeout))) {
                    u16Crc16 = u8RxBuff[FRAME_PACKET_INDEX + u16PacketSize] + ((uint16_t)u8RxBuff[FRAME_PACKET_INDEX + u16PacketSize + 1U] << 8U);
                    if ((uint16_t)CRC_CalculateData8(FRAME_CRC16_INIT_VALUE, &u8RxBuff[FRAME_PACKET_INDEX], u16PacketSize) == u16Crc16) {
                        *u16RxLen = u16PacketSize;
                        return LL_OK;
                    }
                }
            }
        }
    }

    return LL_ERR;
}

/**
 * @brief  Modem send frame.
 * @param  [in] pModemCom               Pointer to a @ref stc_modem_com_t structure
 * @param  [in] u8TxBuff                Pointer to the buffer which the data to be sent
 * @param  [in] u16TxLen                Buffer length
 * @retval None
 */
static void Modem_SendFrame(stc_modem_com_t *pModemCom, uint8_t *u8TxBuff, uint16_t u16TxLen)
{
    uint32_t u32Crc16;

    u8TxBuff[FRAME_LENGTH_INDEX]     = (uint8_t)(u16TxLen & 0xFFU);
    u8TxBuff[FRAME_LENGTH_INDEX + 1] = (uint8_t)((u16TxLen >> 8U) & 0xFFU);
    u32Crc16 = CRC_CalculateData8(FRAME_CRC16_INIT_VALUE, &u8TxBuff[FRAME_PACKET_INDEX], u16TxLen);
    u8TxBuff[FRAME_PACKET_INDEX + u16TxLen]      = (uint8_t)(u32Crc16 & 0xFFU);
    u8TxBuff[FRAME_PACKET_INDEX + u16TxLen + 1U] = (uint8_t)((u32Crc16 >> 8U) & 0xFFU);
    pModemCom->SendData(&u8TxBuff[0], (FRAME_PACKET_INDEX + u16TxLen + 2U));
}

/**
 * @brief  Modem process.
 * @param  [in] pModemCom               Pointer to a @ref stc_modem_com_t structure
 * @param  [in] pModemFlash             Pointer to a @ref stc_modem_flash_t structure
 * @param  [in] u32Timeout              Communication timeout
 * @retval int32_t:
 *           - LL_OK: Communication done
 *           - LL_ERR_TIMEOUT: Communication timeout
 */
int32_t Modem_Process(stc_modem_com_t *pModemCom, stc_modem_flash_t *pModemFlash, uint32_t u32Timeout)
{
    uint32_t u32Temp;
    uint16_t u16PacketSize;
    uint32_t u32TickCnt = 0U;

    for (;;) {
        if (LL_OK == Modem_RecvFrame(pModemCom, &u8FrameData[0], &u16PacketSize, FRAME_RECV_TIMEOUT)) {
            u32TickCnt = 0U;
            u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_OK;
            // Command handle
            switch (u8FrameData[PACKET_CMD_INDEX]) {
                case PACKET_CMD_HANDSHAKE:  /* Reserved */
                    break;
                case PACKET_CMD_APP_UPGRADE:
                    if (0xFFFFFFFFUL != *(__IO uint32_t *)APP_UPGRADE_FLAG_ADDR) {
                        pModemFlash->EraseSector(APP_UPGRADE_FLAG_ADDR, 0U);
                    }
                    u32Temp = APP_UPGRADE_FLAG;
                    if (LL_OK != pModemFlash->WriteData(APP_UPGRADE_FLAG_ADDR, (uint8_t *)&u32Temp, 4U)) {
                        u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ERR;
                    }
                    break;
                default:
                    u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ERR;
                    break;
            }
            Modem_SendFrame(pModemCom, &u8FrameData[0], PACKET_INSTRUCT_SIZE);
            // Check upgrade event
            if ((PACKET_CMD_APP_UPGRADE == u8FrameData[PACKET_CMD_INDEX]) && (PACKET_ACK_OK == u8FrameData[PACKET_RESULT_INDEX])) {
                NVIC_SystemReset();
            }
        } else {
            u32TickCnt += FRAME_RECV_TIMEOUT;
        }
        /* Communication timeout */
        if (u32TickCnt >= u32Timeout) {
            return LL_ERR_TIMEOUT;
        }
    }
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
