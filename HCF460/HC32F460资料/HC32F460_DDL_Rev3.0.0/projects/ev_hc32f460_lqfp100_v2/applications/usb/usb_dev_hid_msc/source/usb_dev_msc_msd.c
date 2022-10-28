/**
 *******************************************************************************
 * @file  usb/usb_dev_hid_msc/source/usb_dev_msc_msd.c
 * @brief user MSC application layer.
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
#include "usb_dev_msc_msd.h"
#include "usb_dev_msc_mem.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Hid_Msc
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
int8_t msc_init(uint8_t lun);
int8_t msc_getcapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size);
int8_t msc_ifready(uint8_t lun);
int8_t msc_ifwrprotected(uint8_t lun);
int8_t msc_read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t msc_write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t msc_getmaxlun(void);

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/* Variable for Storage operation status */
__IO static uint8_t USB_STATUS_REG = 0U;

/* USB Mass storage querty data (36 bytes for each lun) */
const int8_t msc_inquirydata[] = {
    /* LUN 0 */
    0x00,
    0x80,
    0x02,
    0x02,
    (USB_DEV_INQUIRY_LENGTH - 4U),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    'V', 'E', 'N', 'D', 'O', 'R', ' ', ' ', ' ',    /* 9 bytes */
    /* Product Identification */
    'S', 'P', 'I', ' ', 'F', 'l', 'a', 's', 'h',    /* 15 bytes */
    ' ', 'D', 'i', 's', 'k', ' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                             /* 4 bytes */
    /* LUN 1 */
    0x00,
    0x80,
    0x02,
    0x02,
    (USB_DEV_INQUIRY_LENGTH - 4U),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    'V', 'E', 'N', 'D', 'O', 'R', ' ', ' ', ' ',    /* 9 bytes */
    /* Product Identification */
    'S', 'D', ' ', 'F', 'l', 'a', 's', 'h', ' ',    /* 15 bytes */
    'D', 'i', 's', 'k', ' ', ' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                             /* 4 bytes */
};

static USB_DEV_MSC_cbk_TypeDef flash_fops = {
    &msc_init,
    &msc_getcapacity,
    &msc_getmaxlun,
    &msc_ifready,
    &msc_read,
    &msc_write,
    &msc_ifwrprotected,
    (int8_t *)msc_inquirydata
};

/* Pointer to flash_fops */
USB_DEV_MSC_cbk_TypeDef *msc_fops = &flash_fops;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8CopybackBuf[W25Q64_SECTOR_SIZE];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  initialize storage
 * @param  [in] lun          logic number
 * @retval status
 */
int8_t msc_init(uint8_t lun)
{
    BSP_W25QXX_Init();
    return LL_OK;
}

/**
 * @brief  Get Storage capacity
 * @param  [in] lun          logic number
 * @param  [in] block_num    sector number
 * @param  [in] block_size   sector size
 * @retval status
 */
int8_t msc_getcapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    *block_size = 512U;
    *block_num  = W25Q64_MAX_ADDR / 512U;
    return LL_OK;
}

/**
 * @brief  Check if storage is ready
 * @param  [in] lun          logic number
 * @retval status
 */
int8_t  msc_ifready(uint8_t lun)
{
    USB_STATUS_REG |= (uint8_t)0X10;
    return LL_OK;
}

/**
 * @brief  Check if storage is write protected
 * @param  [in] lun          logic number
 * @retval status
 */
int8_t msc_ifwrprotected(uint8_t lun)
{
    return LL_OK;
}

/**
 * @brief  read data from storage devices
 * @param  [in] lun          logic number
 * @param  [in] buf          data buffer be read
 * @param  [in] blk_addr     sector address
 * @param  [in] blk_len      sector count
 * @retval status
 */
int8_t msc_read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res = (int8_t)0;

    USB_STATUS_REG |= 0x02U;
    if (lun == 1U) {
        if (0 != res) {
            USB_STATUS_REG |= 0x08U;
        }
    } else {
        (void)BSP_W25QXX_Read(blk_addr * 512U, buf, (uint32_t)blk_len * 512U);
    }
    return res;
}

static void W25QXX_Write_NoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain = (uint16_t)(256U - WriteAddr % 256U);
    if (NumByteToWrite <= pageremain) {
        pageremain = NumByteToWrite;
    }
    for (;;) {
        (void)BSP_W25QXX_Write(WriteAddr, pBuffer, pageremain);
        if (NumByteToWrite == pageremain) {
            break;
        } else { //NumByteToWrite>pageremain
            pBuffer += pageremain;
            WriteAddr += pageremain;

            NumByteToWrite -= pageremain;
            if (NumByteToWrite > 256U) {
                pageremain = 256U;
            } else {
                pageremain = NumByteToWrite;
            }
        }
    }
}

static void SpiFlashWrite(uint8_t *pbuf, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t *pu8CbBuf;
    pu8CbBuf = u8CopybackBuf;
    secpos = WriteAddr / W25Q64_SECTOR_SIZE;
    secoff = (uint16_t)(WriteAddr % W25Q64_SECTOR_SIZE);
    secremain = (uint16_t)(W25Q64_SECTOR_SIZE - secoff);
    if (NumByteToWrite <= secremain) {
        /* less than 4K */
        secremain = NumByteToWrite;
    }
    for (;;) {
        (void)BSP_W25QXX_Read(secpos * W25Q64_SECTOR_SIZE, pu8CbBuf, W25Q64_SECTOR_SIZE);
        /* check if blank sector */
        for (i = 0U; i < secremain; i++) {
            if (pu8CbBuf[secoff + i] != 0XFFU) {
                break;
            }
        }
        if (i < secremain) {
            /* not blank, need erase */
            (void)BSP_W25QXX_EraseSector(secpos * W25Q64_SECTOR_SIZE);
            /* backup first */
            for (i = 0U; i < secremain; i++) {
                pu8CbBuf[i + secoff] = pbuf[i];
            }
            /* write back after erase */
            W25QXX_Write_NoCheck(pu8CbBuf, secpos * W25Q64_SECTOR_SIZE, (uint16_t)W25Q64_SECTOR_SIZE);

        } else {
            W25QXX_Write_NoCheck(pbuf, WriteAddr, secremain);
        }
        if (NumByteToWrite == secremain) {
            break;
        } else {
            /* next sector */
            secpos++;
            secoff = 0U;

            pbuf += secremain;
            WriteAddr += secremain;
            NumByteToWrite -= secremain;
            if (NumByteToWrite > W25Q64_SECTOR_SIZE) {
                secremain = (uint16_t)W25Q64_SECTOR_SIZE;
            } else {
                secremain = NumByteToWrite;
            }
        }
    }
}


/**
 * @brief  Write data to storage devices
 * @param  [in] lun          logic number
 * @param  [in] buf          data buffer be written
 * @param  [in] blk_addr     sector address
 * @param  [in] blk_len      sector count
 * @retval status
 */
int8_t msc_write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res = (int8_t)0;

    USB_STATUS_REG |= 0X01U;
    if (lun == 1U) {
        if (0 != res) {
            USB_STATUS_REG |= 0X04U;
        }
    } else {
        SpiFlashWrite(buf, blk_addr * 512U, blk_len * 512U);
    }
    return res;
}

/**
 * @brief  Get supported logic number
 * @param  None
 * @retval 1
 */
int8_t msc_getmaxlun(void)
{
    return 1;
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
