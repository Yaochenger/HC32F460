/**
 *******************************************************************************
 * @file  sdioc/sdioc_mmc/source/mmc.h
 * @brief This file contains all the functions prototypes of the Secure
 *        Digital(MMC) driver library.
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
#ifndef __MMC_H__
#define __MMC_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_sdioc.h"
#include "hc32_ll_dma.h"
#include "hc32_ll_utility.h"

/**
 * @addtogroup HC32F4A0_DDL_Examples
 * @{
 */

/**
 * @addtogroup SDIOC_MMC_Card
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 * @defgroup MMC_Global_Types MMC Global Types
 * @{
 */

/**
 * @brief MMC Card State enumeration structure definition
 */
typedef enum {
    MMC_CARD_STAT_IDLE       = 0x00U,   /*!< Card state is idle                      */
    MMC_CARD_STAT_RDY        = 0x01U,   /*!< Card state is ready                     */
    MMC_CARD_STAT_IDENTIFY   = 0x02U,   /*!< Card is in identification state         */
    MMC_CARD_STAT_STANDBY    = 0x03U,   /*!< Card is in standby state                */
    MMC_CARD_STAT_TRANS      = 0x04U,   /*!< Card is in transfer state               */
    MMC_CARD_STAT_TX_DATA    = 0x05U,   /*!< Card is sending an operation            */
    MMC_CARD_STAT_RX_DATA    = 0x06U,   /*!< Card is receiving operation information */
    MMC_CARD_STAT_PAG        = 0x07U,   /*!< Card is in programming state            */
    MMC_CARD_STAT_DISCONNECT = 0x08U    /*!< Card is disconnected                    */
} en_mmc_card_state_t;

/**
 * @brief MMC Card Information Structure definition
 */
typedef struct {
    uint32_t u32CardType;               /*!< Specifies the card Type                       */
    uint32_t u32Class;                  /*!< Specifies the class of the card class         */
    uint32_t u32RelativeCardAddr;       /*!< Specifies the Relative Card Address           */
    uint32_t u32BlockNum;               /*!< Specifies the Card Capacity in blocks         */
    uint32_t u32BlockSize;              /*!< Specifies one block size in bytes             */
    uint32_t u32LogBlockNum;            /*!< Specifies the Card logical Capacity in blocks */
    uint32_t u32LogBlockSize;           /*!< Specifies logical block size in bytes         */
} stc_mmc_card_info_t;

/**
 * @brief MMC Card Specific Data(CSD Register) Structure definition
 */
typedef struct {
    uint8_t  u8CSDStruct;               /*!< CSD structure                         */
    uint8_t  u8SysSpecVersion;          /*!< System specification version          */
    uint8_t  u8Reserved1;               /*!< Reserved                              */
    uint8_t  u8TAAC;                    /*!< Data read access time 1               */
    uint8_t  u8NSAC;                    /*!< Data read access time 2 in CLK cycles */
    uint8_t  u8MaxBusClkFreq;           /*!< Max. bus clock frequency              */
    uint16_t u16CardCmdClass;           /*!< Card command classes                  */
    uint8_t  u8ReadBlockLen;            /*!< Max. read data block length           */
    uint8_t  u8BlockReadPartial;        /*!< Partial blocks for read allowed       */
    uint8_t  u8WriteBlockMisalign;      /*!< Write block misalignment              */
    uint8_t  u8ReadBlockMisalign;       /*!< Read block misalignment               */
    uint8_t  u8DSRImplement;            /*!< DSR implemented                       */
    uint8_t  u8Reserved2;               /*!< Reserved                              */
    uint32_t u32DeviceSize;             /*!< Device Size                           */
    uint8_t  u8MaxReadCurrVDDMin;       /*!< Max. read current @ VDD min           */
    uint8_t  u8MaxReadCurrVDDMax;       /*!< Max. read current @ VDD max           */
    uint8_t  u8MaxWriteCurrVDDMin;      /*!< Max. write current @ VDD min          */
    uint8_t  u8MaxWriteCurrVDDMax;      /*!< Max. write current @ VDD max          */
    uint8_t  u8DeviceSizeMul;           /*!< Device size multiplier                */
    uint8_t  u8EraseGroupSize;          /*!< Erase group size                      */
    uint8_t  u8EraseGroupSizeMul;       /*!< Erase group size multiplier           */
    uint8_t  u8WriteProtectGroupSize;   /*!< Write protect group size              */
    uint8_t  u8WriteProtectGroupEn;     /*!< Write protect group enable            */
    uint8_t  u8ManuDefaultECC;          /*!< Manufacturer default ECC              */
    uint8_t  u8WriteSpeedFactor;        /*!< Write speed factor                    */
    uint8_t  u8MaxWriteBlockLen;        /*!< Max. write data block length          */
    uint8_t  u8WriteBlockPartial;       /*!< Partial blocks for write allowed      */
    uint8_t  u8Reserved3;               /*!< Reserved                              */
    uint8_t  u8ContProtectApp;          /*!< Content protection application        */
    uint8_t  u8FileFormatGroup;         /*!< File format group                     */
    uint8_t  u8CopyFlag;                /*!< Copy flag (OTP)                       */
    uint8_t  u8PermWriteProtect;        /*!< Permanent write protection            */
    uint8_t  u8TempWriteProtect;        /*!< Temporary write protection            */
    uint8_t  u8FileFormat;              /*!< File format                           */
    uint8_t  u8ECCCode;                 /*!< ECC code                              */
    uint8_t  u8CRCChecksum;             /*!< CSD CRC7 checksum                     */
    uint8_t  u8Reserved4;               /*!< Always 1                              */
} stc_mmc_card_csd_t;

/**
 * @brief MMC Card Identification Data(CID Register) Structure definition
 */
typedef struct {
    uint8_t  u8ManufacturerID;          /*!< Manufacturer ID       */
    uint8_t  u8Reserved1;               /*!< Reserved              */
    uint8_t  u8DeviceBGA;               /*!< Device/BGA            */
    uint16_t u16OemAppID;               /*!< OEM/Application ID    */
    uint32_t u32ProductName1;           /*!< Product Name part1    */
    uint16_t u16ProductName2;           /*!< Product Name part2    */
    uint8_t  u8ProductRevision;         /*!< Product Revision      */
    uint32_t u32ProductSN;              /*!< Product Serial Number */
    uint8_t  u8ManufactDate;            /*!< Manufacturing Date    */
    uint8_t  u8CRCChecksum;             /*!< CRC7 checksum         */
    uint8_t  u8Reserved2;               /*!< Always 1              */
} stc_mmc_card_cid_t;

/**
 * @brief MMC handle Structure definition
 */
typedef struct {
    CM_SDIOC_TypeDef    *SDIOCx;        /*!< Pointer to SDIOC registers base address            */
    stc_sdioc_init_t    stcSdiocInit;   /*!< SDIOC Initialize structure @ref stc_sdioc_init_t   */
    CM_DMA_TypeDef      *DMAx;          /*!< Pointer to DMA registers base address              */
    uint8_t             u8DmaTxCh;      /*!< Specifies the DMA channel used to send             */
    uint8_t             u8DmaRxCh;      /*!< Specifies the DMA channel used to receive          */
    uint8_t             *pu8Buffer;     /*!< Pointer to SD Tx/Rx transfer Buffer                */
    uint32_t            u32Len;         /*!< SD Tx/Rx Transfer length                           */
    uint32_t            u32Context;     /*!< MMC transfer context                               */
    uint32_t            u32ErrorCode;   /*!< MMC Card Error codes                               */
    stc_mmc_card_info_t stcMmcCardInfo; /*!< MMC Card information                               */
    uint32_t            u32CSD[4];      /*!< MMC card specific data table                       */
    uint32_t            u32CID[4];      /*!< MMC card identification number table               */
} stc_mmc_handle_t;

/**
 * @}
 */

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup MMC_Global_Macros MMC Global Macros
 * @{
 */

/**
 * @defgroup MMC_Transfer_Context MMC Transfer Context
 * @{
 */
#define MMC_CONTEXT_NONE                        (0x00UL)    /*!< None                            */
#define MMC_CONTEXT_RD_SINGLE_BLOCK             (0x01UL)    /*!< Read single block operation     */
#define MMC_CONTEXT_RD_MULTI_BLOCK              (0x02UL)    /*!< Read multiple blocks operation  */
#define MMC_CONTEXT_WR_SINGLE_BLOCK             (0x10UL)    /*!< Write single block operation    */
#define MMC_CONTEXT_WR_MULTI_BLOCK              (0x20UL)    /*!< Write multiple blocks operation */
#define MMC_CONTEXT_INT                         (0x40UL)    /*!< Process in Interrupt mode       */
#define MMC_CONTEXT_DMA                         (0x80UL)    /*!< Process in DMA mode             */
/**
 * @}
 */

/**
 * @defgroup MMC_Voltage_Mode MMC Voltage Mode
 * @{
 */
#define MMC_VOLT_RANGE_HIGH_VOLT                (0x80FF8000UL)  /*!< VALUE OF ARGUMENT          */
#define MMC_VOLT_RANGE_DUAL_VOLT                (0x80FF8080UL)  /*!< VALUE OF ARGUMENT          */
#define EMMC_VOLT_RANGE_HIGH_VOLT               (0xC0FF8000UL)  /*!< For eMMC > 2Gb sector mode */
#define EMMC_VOLT_RANGE_DUAL_VOLT               (0xC0FF8080UL)  /*!< For eMMC > 2Gb sector mode */
#define MMC_VOLT_RANGE_INVD_VOLT                (0x0001FF01UL)  /*!< Invalid voltage            */
/**
 * @}
 */

/**
 * @defgroup MMC_Memory_Card MMC Memory Card
 * @{
 */
#define  MMC_HIGH_VOLT_CARD                     (0x00000000UL)
#define  MMC_DUAL_VOLT_CARD                     (0x00000001UL)
/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup MMC_Global_Functions
 * @{
 */
int32_t MMC_DeInit(stc_mmc_handle_t *handle);
int32_t MMC_Init(stc_mmc_handle_t *handle);
int32_t MMC_GetCardState(stc_mmc_handle_t *handle, en_mmc_card_state_t *peCardState);
int32_t MMC_GetCardCID(const stc_mmc_handle_t *handle, stc_mmc_card_cid_t *pstcCardCID);
int32_t MMC_GetCardCSD(stc_mmc_handle_t *handle, stc_mmc_card_csd_t *pstcCardCSD);
int32_t MMC_GetCardInfo(stc_mmc_handle_t *handle, stc_mmc_card_info_t *pstcCardInfo);
int32_t MMC_GetErrorCode(const stc_mmc_handle_t *handle, uint32_t *pu32ErrorCode);

/* Callback in non blocking modes */
void MMC_IRQHandler(stc_mmc_handle_t *handle);
void MMC_TxCompleteCallback(stc_mmc_handle_t *handle);
void MMC_RxCompleteCallback(stc_mmc_handle_t *handle);
void MMC_ErrorCallback(stc_mmc_handle_t *handle);

/* Polling Mode */
int32_t MMC_Erase(stc_mmc_handle_t *handle, uint32_t u32BlockStartAddr, uint32_t u32BlockEndAddr);
int32_t MMC_ReadBlocks(stc_mmc_handle_t *handle, uint32_t u32BlockAddr, uint16_t u16BlockCount,
                       uint8_t *pu8Data, uint32_t u32Timeout);
int32_t MMC_WriteBlocks(stc_mmc_handle_t *handle, uint32_t u32BlockAddr, uint16_t u16BlockCount,
                        uint8_t *pu8Data, uint32_t u32Timeout);
/* Interrupt Mode */
int32_t MMC_ReadBlocks_INT(stc_mmc_handle_t *handle, uint32_t u32BlockAddr, uint16_t u16BlockCount, uint8_t *pu8Data);
int32_t MMC_WriteBlocks_INT(stc_mmc_handle_t *handle, uint32_t u32BlockAddr, uint16_t u16BlockCount, uint8_t *pu8Data);
/* DMA Mode */
int32_t MMC_ReadBlocks_DMA(stc_mmc_handle_t *handle, uint32_t u32BlockAddr, uint16_t u16BlockCount, uint8_t *pu8Data);
int32_t MMC_WriteBlocks_DMA(stc_mmc_handle_t *handle, uint32_t u32BlockAddr, uint16_t u16BlockCount, uint8_t *pu8Data);
/* Abort */
int32_t MMC_Abort(stc_mmc_handle_t *handle);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __MMC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
