/**
 *******************************************************************************
 * @file  iap/iap_app/source/ymodem.c
 * @brief This file provides firmware functions to manage the ymodem.
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
#include "ymodem.h"
#include "flash.h"
#include "com.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Packet status definition */
#define PACKET_END_TRANS                (0U)
#define PACKET_ABORT_SENDER             (2U)

/* Character covert */
#define IS_UPPERCASE_LETTER(c)          (((c) >= 'A') && ((c) <= 'F'))
#define IS_LOWERCASE_LETTER(c)          (((c) >= 'a') && ((c) <= 'f'))
#define IS_BASE_NUM(c)                  (((c) >= '0') && ((c) <= '9'))
#define IS_VALID_HEX(c)                 (IS_UPPERCASE_LETTER(c) || IS_LOWERCASE_LETTER(c) || IS_BASE_NUM(c))
#define IS_VALID_DEC(c)                 IS_BASE_NUM(c)
#define ASCII_CONVERT_DEC(c)            ((c) - '0')
#define CONVERT_HEX_MID(c)              (IS_UPPERCASE_LETTER(c) ? ((c) - 'A' + 10) : ((c) - 'a' + 10))
#define ASCII_CONVERT_HEX(c)            (IS_BASE_NUM(c) ? ASCII_CONVERT_DEC(c) : CONVERT_HEX_MID(c))

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
__ALIGN_BEGIN uint8_t u8PacketData[PACKET_STX_SIZE + PACKET_DATA_INDEX + PACKET_CRC_SIZE];

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
 * @brief  Convert an Integer to a string.
 * @param  pu8Str                       The string output pointer.
 * @param  u32Value                     The integer to be converted.
 * @retval None
 */
void YModem_Int2Str(uint8_t *pu8Str, uint32_t u32Value)
{
    uint32_t i, divider = 1000000000;
    uint32_t pos = 0, mark = 0;

    for (i = 0; i < 10; i++) {
        pu8Str[pos++] = (u32Value / divider) + '0';
        if ((pu8Str[pos - 1] == '0') & (mark == 0)) {
            pos = 0;
        } else {
            mark++;
        }
        u32Value %= divider;
        divider /= 10;
    }
}

/**
 * @brief  Convert a string to an integer.
 * @param  pu32Value                    The integer value pointer.
 * @param  pu8Str                       The string to be converted.
 * @retval int32_t:
 *           - LL_OK: Convert done
 *           - LL_ERR: Convert error
 */
int32_t YModem_Str2Int(uint32_t *pu32Value, uint8_t *pu8Str)
{
    int32_t i32Ret = LL_ERR;
    uint32_t i = 0;
    uint32_t val = 0;

    if ((pu8Str[0] == '0') && ((pu8Str[1] == 'x') || (pu8Str[1] == 'X'))) {
        i = 2;
        while ((i < 11) && (pu8Str[i] != '\0')) {
            if (IS_VALID_HEX(pu8Str[i])) {
                val = (val << 4) + ASCII_CONVERT_HEX(pu8Str[i]);
            } else {
                i32Ret = LL_ERR;    /* Invalid input */
                break;
            }
            i++;
        }
        if (pu8Str[i] == '\0') {    /* valid i32Ret */
            *pu32Value = val;
            i32Ret = LL_OK;
        }
    } else {
        /* Max 10-digit decimal input */
        while ((i < 11) && (i32Ret != LL_OK)) {
            if (pu8Str[i] == '\0') {
                *pu32Value = val;
                i32Ret = LL_OK;
            } else if (((pu8Str[i] == 'k') || (pu8Str[i] == 'K')) && (i > 0)) {
                val = val << 10;
                *pu32Value = val;
                i32Ret = LL_OK;
            } else if (((pu8Str[i] == 'm') || (pu8Str[i] == 'M')) && (i > 0)) {
                val = val << 20;
                *pu32Value = val;
                i32Ret = LL_OK;
            } else if (IS_VALID_DEC(pu8Str[i])) {
                val = val * 10 + ASCII_CONVERT_DEC(pu8Str[i]);
            } else {
                i32Ret = LL_ERR;
                break;
            }
            i++;
        }
    }

    return i32Ret;
}

/**
 * @brief  Update CRC16 for input byte.
 * @param  crcIn                        Input value
 * @param  byte                         Input byte
 * @retval None
 */
static uint16_t CRC16_Update(uint16_t crcIn, uint8_t byte)
{
    uint32_t crc = crcIn;
    uint32_t in = byte | 0x100;

    do {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100) {
            ++crc;
        }
        if (crc & 0x10000) {
            crc ^= 0x1021;
        }
    } while (!(in & 0x10000));

    return crc & 0xFFFFU;
}

/**
 * @brief  Cal CRC16 for YModem Packet.
 * @param  pu8Data
 * @param  size
 * @retval CRC16 value.
 */
static uint16_t CRC16_CalData(const uint8_t *pu8Data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t *dataEnd = pu8Data + size;

    while (pu8Data < dataEnd) {
        crc = CRC16_Update(crc, *pu8Data++);
    }
    crc = CRC16_Update(crc, 0);
    crc = CRC16_Update(crc, 0);

    return crc & 0xFFFFU;
}

/**
 * @brief  Prepare the first packet.
 * @param  pu8Data                      output buffer
 * @param  pu8FileName                  name of the file to be sent
 * @param  u32Len                       length of the file to be sent in bytes
 * @retval None
 */
static void Prepare_InitPacket(uint8_t *pu8Data, const uint8_t *pu8FileName, uint32_t u32Len)
{
    uint32_t i, j = 0;
    uint8_t u8Str[FILE_SIZE_LEN];

    /* First 3 bytes are constant */
    pu8Data[PACKET_START_INDEX]  = YMODEM_SOH;
    pu8Data[PACKET_NUM_INDEX]    = 0x00;
    pu8Data[PACKET_XORNUM_INDEX] = 0xFF;
    /* Filename written */
    for (i = 0; (pu8FileName[i] != '\0') && (i < FILE_NAME_LEN); i++) {
        pu8Data[i + PACKET_DATA_INDEX] = pu8FileName[i];
    }
    pu8Data[i + PACKET_DATA_INDEX] = 0x00;
    /* File size written */
    YModem_Int2Str(u8Str, u32Len);
    i += PACKET_DATA_INDEX + 1;
    while (u8Str[j] != '\0') {
        pu8Data[i++] = u8Str[j++];
    }
    /* Padding with zeros */
    for (j = i; j < PACKET_SOH_SIZE + PACKET_DATA_INDEX; j++) {
        pu8Data[j] = 0;
    }
}

/**
 * @brief  Prepare the data packet.
 * @param  pu8Src                       Pointer to the data to be sent
 * @param  pu8Packet                    Pointer to the output buffer
 * @param  u8Num                        Number of the packet
 * @param  u32Size                      Length of the packet to be sent in bytes
 * @retval None
 */
static void Prepare_Packet(uint8_t *pu8Src, uint8_t *pu8Packet, uint8_t u8Num, uint32_t u32Size)
{
    uint8_t *pu8Data;
    uint32_t i, dataSize, packetSize;

    pu8Data = pu8Src;
    /* Make packet */
    packetSize = u32Size >= PACKET_STX_SIZE ? PACKET_STX_SIZE : PACKET_SOH_SIZE;
    dataSize = u32Size < packetSize ? u32Size : packetSize;
    if (packetSize == PACKET_STX_SIZE) {
        pu8Packet[PACKET_START_INDEX] = YMODEM_STX;
    } else {
        pu8Packet[PACKET_START_INDEX] = YMODEM_SOH;
    }
    pu8Packet[PACKET_NUM_INDEX]    = u8Num;
    pu8Packet[PACKET_XORNUM_INDEX] = (~u8Num);
    /* Filename packet has valid data */
    for (i = PACKET_DATA_INDEX; i < dataSize + PACKET_DATA_INDEX; i++) {
        pu8Packet[i] = *pu8Data++;
    }
    if (dataSize <= packetSize) {
        for (i = dataSize + PACKET_DATA_INDEX; i < packetSize + PACKET_DATA_INDEX; i++) {
            pu8Packet[i] = 0x1A;    /* (0x1A) or 0x00 */
        }
    }
}

/**
 * @brief  Receive a packet from sender.
 * @param  pu8Data
 * @param  pu32Len
 *           @arg PACKET_END_TRANS:     End of transmission
 *           @arg PACKET_ABORT_SENDER:  Abort by sender
 *           @arg other:                Packet length
 * @param  timeout
 * @retval int32_t:
 *           - LL_OK: Receive done
 *           - LL_ERR: Receive error
 *           - LL_ERR_BUSY: Receive busy
 */
static int32_t ReceivePacket(uint8_t *pu8Data, uint32_t *pu32Len, uint32_t timeout)
{
    uint32_t u32Crc;
    uint32_t packetSize = PACKET_END_TRANS;
    int32_t i32Ret;
    uint8_t u8Temp;

    *pu32Len = 0;
    i32Ret = COM_RecvData(&u8Temp, 1, timeout);
    if (i32Ret == LL_OK) {
        switch (u8Temp) {
            case YMODEM_SOH:
                packetSize = PACKET_SOH_SIZE;
                break;
            case YMODEM_STX:
                packetSize = PACKET_STX_SIZE;
                break;
            case YMODEM_EOT:
                break;
            case YMODEM_CAN:
                if ((COM_RecvData(&u8Temp, 1, timeout) == LL_OK) && (u8Temp == YMODEM_CAN)) {
                    packetSize = PACKET_ABORT_SENDER;
                } else {
                    i32Ret = LL_ERR;
                }
                break;
            case YMODEM_ABORT1:
            case YMODEM_ABORT2:
                i32Ret = LL_ERR_BUSY;
                break;
            default:
                i32Ret = LL_ERR;
                break;
        }
        pu8Data[PACKET_START_INDEX] = u8Temp;
        if (packetSize >= PACKET_SOH_SIZE) {
            i32Ret = COM_RecvData(&pu8Data[PACKET_NUM_INDEX], packetSize + PACKET_OVERHEAD_SIZE, timeout);
            /* Packet sanity check */
            if (i32Ret == LL_OK) {
                if (pu8Data[PACKET_NUM_INDEX] != ((pu8Data[PACKET_XORNUM_INDEX]) ^ YMODEM_NUM_XOR_BYTE)) {
                    packetSize = PACKET_END_TRANS;
                    i32Ret = LL_ERR;
                } else {
                    /* Check packet CRC */
                    u32Crc = pu8Data[packetSize + PACKET_DATA_INDEX] << 8;
                    u32Crc += pu8Data[packetSize + PACKET_DATA_INDEX + 1];
                    if (CRC16_CalData(&pu8Data[PACKET_DATA_INDEX], packetSize) != u32Crc) {
                        packetSize = PACKET_END_TRANS;
                        i32Ret = LL_ERR;
                    }
                }
            } else {
                packetSize = PACKET_END_TRANS;
            }
        }
    }
    *pu32Len = packetSize;

    return i32Ret;
}

/**
 * @brief  Transmit a file using the ymodem protocol.
 * @param  pu8Buf                       Address of the first byte
 * @param  pu8FileName                  Name of the file sent
 * @param  fileSize                     Size of the transmission
 * @retval int32_t:
 *           - YMODEM_COM_OK
 *           - YMODEM_COM_ERR
 *           - YMODEM_COM_ABORT
 *           - YMODEM_COM_LIMIT
 */
int32_t YModem_Transmit(uint8_t *pu8Buf, const uint8_t *pu8FileName, uint32_t fileSize)
{
    uint8_t i, temp = 0;
    uint8_t *pu8TxBuf;
    uint32_t errCnt = 0, ackFlag = 0, packetSize;
    uint32_t packetNum = 1, crcValue;
    int32_t i32Ret = YMODEM_COM_ERR;

    /* Wait for 'C' */
    COM_RecvData(&temp, 1, YMODEM_RECV_WAITFOREVER);
    if (temp == YMODEM_CRC16) {
        pu8TxBuf = pu8Buf;
        i32Ret = YMODEM_COM_OK;
        /* Prepare first packet */
        Prepare_InitPacket(u8PacketData, pu8FileName, fileSize);
        while ((!ackFlag) && (i32Ret == YMODEM_COM_OK)) {
            /* Send Packet */
            crcValue = CRC16_CalData(&u8PacketData[PACKET_DATA_INDEX], PACKET_SOH_SIZE);
            u8PacketData[PACKET_SOH_SIZE + PACKET_DATA_INDEX]     = (uint8_t)(crcValue >> 8);
            u8PacketData[PACKET_SOH_SIZE + PACKET_DATA_INDEX + 1] = (uint8_t)(crcValue & 0xFF);
            COM_SendData(&u8PacketData[PACKET_START_INDEX], PACKET_SOH_SIZE + PACKET_HEAD_SIZE + PACKET_CRC_SIZE);
            /* Wait for Ack and 'C' */
            if (COM_RecvData(&temp, 1, YMODEM_NAK_TIMEOUT) == LL_OK) {
                if (temp == YMODEM_ACK) {
                    ackFlag = 1;
                } else if (temp == YMODEM_CAN) {
                    if ((COM_RecvData(&temp, 1, YMODEM_NAK_TIMEOUT) == LL_OK) && (temp == YMODEM_CAN)) {
                        i32Ret = YMODEM_COM_ABORT;
                    }
                }
            } else {
                errCnt++;
            }
            if (errCnt >= YMODEM_MAX_ERR) {
                i32Ret = YMODEM_COM_ERR;
            }
        }
        /* Send the packets */
        while ((fileSize) && (i32Ret == YMODEM_COM_OK)) {
            /* Prepare next packet */
            Prepare_Packet(pu8TxBuf, u8PacketData, packetNum, fileSize);
            ackFlag = 0;
            errCnt = 0;
            while ((!ackFlag) && (i32Ret == YMODEM_COM_OK)) {
                /* Send next packet */
                if (fileSize >= PACKET_STX_SIZE) {
                    packetSize = PACKET_STX_SIZE;
                } else {
                    packetSize = PACKET_SOH_SIZE;
                }
                crcValue = CRC16_CalData(&u8PacketData[PACKET_DATA_INDEX], packetSize);
                u8PacketData[packetSize + PACKET_DATA_INDEX]     = (uint8_t)(crcValue >> 8);
                u8PacketData[packetSize + PACKET_DATA_INDEX + 1] = (uint8_t)(crcValue & 0xFF);
                COM_SendData(&u8PacketData[PACKET_START_INDEX], packetSize + PACKET_HEAD_SIZE + PACKET_CRC_SIZE);
                /* Wait for Ack */
                if ((COM_RecvData(&temp, 1, YMODEM_NAK_TIMEOUT) == LL_OK) && (temp == YMODEM_ACK)) {
                    ackFlag = 1;
                    if (fileSize > packetSize) {
                        pu8TxBuf += packetSize;
                        fileSize -= packetSize;
                        if (packetNum == (IAP_APP_SIZE / PACKET_STX_SIZE)) {
                            i32Ret = YMODEM_COM_LIMIT;
                        } else {
                            packetNum++;
                        }
                    } else {
                        pu8TxBuf += packetSize;
                        fileSize = 0;
                    }
                } else {
                    errCnt++;
                }
                /* Max retry 5 times */
                if (errCnt >= YMODEM_MAX_ERR) {
                    i32Ret = YMODEM_COM_ERR;
                }
            }
        }
        /* Sending End Of Transmission char */
        ackFlag = 0;
        errCnt = 0;
        while ((!ackFlag) && (i32Ret == YMODEM_COM_OK)) {
            temp = YMODEM_EOT;
            COM_SendData(&temp, 1);
            /* Wait for Ack */
            if (COM_RecvData(&temp, 1, YMODEM_NAK_TIMEOUT) == LL_OK) {
                if (temp == YMODEM_ACK) {
                    ackFlag = 1;
                } else if (temp == YMODEM_CAN) {
                    if ((COM_RecvData(&temp, 1, YMODEM_NAK_TIMEOUT) == LL_OK) && (temp == YMODEM_CAN)) {
                        i32Ret = YMODEM_COM_ABORT;
                    }
                }
            } else {
                errCnt++;
            }
            if (errCnt >=  YMODEM_MAX_ERR) {
                i32Ret = YMODEM_COM_ERR;
            }
        }
        /* Empty packet sent - some terminal emulators need this to close session */
        if (i32Ret == YMODEM_COM_OK) {
            /* Preparing an empty packet */
            u8PacketData[PACKET_START_INDEX]  = YMODEM_SOH;
            u8PacketData[PACKET_NUM_INDEX]    = 0;
            u8PacketData[PACKET_XORNUM_INDEX] = 0xFF;
            for (i = PACKET_DATA_INDEX; i < (PACKET_SOH_SIZE + PACKET_DATA_INDEX); i++) {
                u8PacketData[i] = 0;
            }
            /* Send Packet */
            crcValue = CRC16_CalData(&u8PacketData[PACKET_DATA_INDEX], PACKET_SOH_SIZE);
            u8PacketData[PACKET_SOH_SIZE + PACKET_DATA_INDEX]     = (uint8_t)(crcValue >> 8);
            u8PacketData[PACKET_SOH_SIZE + PACKET_DATA_INDEX + 1] = (uint8_t)(crcValue & 0xFF);
            COM_SendData(&u8PacketData[PACKET_START_INDEX], PACKET_SOH_SIZE + PACKET_HEAD_SIZE + PACKET_CRC_SIZE);
            /* Wait for Ack and 'C' */
            if (COM_RecvData(&temp, 1, YMODEM_NAK_TIMEOUT) == LL_OK) {
                if (temp == YMODEM_CAN) {
                    i32Ret = YMODEM_COM_ABORT;
                }
            }
        }
    }

    return i32Ret;
}

/**
 * @brief  Receive a file using the ymodem protocol with CRC16.
 * @param  pu8FileName                  Name of the file receive
 * @param  pu32Size                     The size of the file
 * @retval int32_t:
 *           - YMODEM_COM_OK
 *           - YMODEM_COM_ERR
 *           - YMODEM_COM_ABORT
 *           - YMODEM_COM_LIMIT
 *           - YMODEM_COM_FLASH_ERR
 */
int32_t YModem_Receive(uint8_t *pu8FileName, uint32_t *pu32Size)
{
    __IO uint32_t appFlashAddr;
    uint32_t i, packetLen, errCnt = 0;
    uint32_t sessionBegin = 0, sessionDone = 0, fileDone;
    uint32_t fileSize, packetCnt;
    uint8_t *filePtr;
    uint8_t fileSizeStr[FILE_SIZE_LEN], temp = 0;
    int32_t i32Ret = YMODEM_COM_OK;

    /* Initialize appFlashAddr variable */
    appFlashAddr = IAP_APP_ADDR;
    while ((sessionDone == 0) && (i32Ret == YMODEM_COM_OK)) {
        packetCnt = 0;
        fileDone = 0;
        while ((fileDone == 0) && (i32Ret == YMODEM_COM_OK)) {
            switch (ReceivePacket(u8PacketData, &packetLen, YMODEM_RECV_TIMEOUT)) {
                case LL_OK:
                    errCnt = 0;
                    switch (packetLen) {
                        case 2: /* Abort by sender */
                            temp = YMODEM_ACK;
                            COM_SendData(&temp, 1);
                            i32Ret = YMODEM_COM_ABORT;
                            break;
                        case 0: /* End of transmission */
                            temp = YMODEM_ACK;
                            COM_SendData(&temp, 1);
                            fileDone = 1;
                            i = APP_EXIST_FLAG;
                            FLASH_WriteData(APP_EXIST_FLAG_ADDR, (uint8_t *)&i, 4U);
                            break;
                        default: /* Normal packet */
                            if (u8PacketData[PACKET_NUM_INDEX] != (uint8_t)packetCnt) {
                                temp = YMODEM_NAK;
                                COM_SendData(&temp, 1);
                            } else {
                                if (packetCnt == 0) {
                                    /* File name packet */
                                    if (u8PacketData[PACKET_DATA_INDEX] != 0) {
                                        /* File name extraction */
                                        i = 0;
                                        filePtr = u8PacketData + PACKET_DATA_INDEX;
                                        while ((*filePtr != 0) && (i < FILE_NAME_LEN)) {
                                            pu8FileName[i++] = *filePtr++;
                                        }
                                        pu8FileName[i++] = '\0';
                                        /* File size extraction */
                                        i = 0;
                                        filePtr++;
                                        while ((*filePtr != ' ') && (i < FILE_SIZE_LEN)) {
                                            fileSizeStr[i++] = *filePtr++;
                                        }
                                        fileSizeStr[i++] = '\0';
                                        YModem_Str2Int(&fileSize, fileSizeStr);
                                        /* Image size is greater than Flash size */
                                        if (fileSize > IAP_APP_SIZE) {
                                            temp = YMODEM_CAN;  /* End session */
                                            COM_SendData(&temp, 1);
                                            COM_SendData(&temp, 1);
                                            i32Ret = YMODEM_COM_LIMIT;
                                        } else {
                                            /* Erase user application area */
                                            FLASH_EraseSector(IAP_APP_ADDR, fileSize);
                                            FLASH_EraseSector(APP_EXIST_FLAG_ADDR, 0U);
                                            *pu32Size = fileSize;
                                            temp = YMODEM_ACK;
                                            COM_SendData(&temp, 1);
                                            temp = YMODEM_CRC16;
                                            COM_SendData(&temp, 1);
                                        }
                                    } else { /* File header packet is empty, end session */
                                        temp = YMODEM_ACK;
                                        COM_SendData(&temp, 1);
                                        fileDone = 1;
                                        sessionDone = 1;
                                        break;
                                    }
                                } else { /* Data packet */
                                    if (FLASH_WriteData(appFlashAddr, &u8PacketData[PACKET_DATA_INDEX], packetLen) == LL_OK) {
                                        appFlashAddr += packetLen;
                                        temp = YMODEM_ACK;
                                        COM_SendData(&temp, 1);
                                    } else {    /* End session */
                                        temp = YMODEM_CAN;
                                        COM_SendData(&temp, 1);
                                        COM_SendData(&temp, 1);
                                        i32Ret = YMODEM_COM_FLASH_ERR;
                                    }
                                }
                                packetCnt++;
                                sessionBegin = 1;
                            }
                            break;
                    }
                    break;
                case LL_ERR_BUSY: /* Abort actually */
                    temp = YMODEM_CAN;
                    COM_SendData(&temp, 1);
                    COM_SendData(&temp, 1);
                    i32Ret = YMODEM_COM_ABORT;
                    break;
                default:
                    if (sessionBegin > 0) {
                        errCnt++;
                    }
                    if (errCnt > YMODEM_MAX_ERR) {
                        temp = YMODEM_CAN;  /* Abort communication */
                        COM_SendData(&temp, 1);
                        COM_SendData(&temp, 1);
                    } else {
                        temp = YMODEM_CRC16;  /* Ask for a packet */
                        COM_SendData(&temp, 1);
                    }
                    break;
            }
        }
    }

    return i32Ret;
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
