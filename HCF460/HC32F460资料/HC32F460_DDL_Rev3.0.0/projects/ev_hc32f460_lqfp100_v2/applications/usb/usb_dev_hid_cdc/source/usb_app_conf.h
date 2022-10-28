/**
 *******************************************************************************
 * @file  usb/usb_dev_hid_cdc/source/usb_app_conf.h
 * @brief low level driver configuration
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
#ifndef __USB_APP_CONF_H__
#define __USB_APP_CONF_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/

/* USB MODE CONFIGURATION */
#define USB_FS_MODE
#define USE_DEVICE_MODE

#ifndef USB_FS_MODE
#define USB_FS_MODE
#endif

#ifndef USE_DEVICE_MODE
#ifndef USE_HOST_MODE
#error  "USE_DEVICE_MODE or USE_HOST_MODE should be defined"
#endif
#endif

/* USB DEVICE ENDPOINT CONFIGURATION */
#define CDC_IN_EP               (0x84U)
#define CDC_OUT_EP              (0x05U)
#define CDC_CMD_EP              (0x83U)

#define HID_IN_EP               (0x81U)
#define HID_OUT_EP              (0x02U)

/* USB FIFO CONFIGURATION */
#define RX_FIFO_FS_SIZE         (128U)
#define TX0_FIFO_FS_SIZE        (64U)
#define TX1_FIFO_FS_SIZE        (64U)
#define TX2_FIFO_FS_SIZE        (0U)
#define TX3_FIFO_FS_SIZE        (32U)
#define TX4_FIFO_FS_SIZE        (32U)
#define TX5_FIFO_FS_SIZE        (0U)

#if ((RX_FIFO_FS_SIZE + TX0_FIFO_FS_SIZE + TX1_FIFO_FS_SIZE + TX2_FIFO_FS_SIZE + TX3_FIFO_FS_SIZE + TX4_FIFO_FS_SIZE + \
      TX5_FIFO_FS_SIZE) > 320U)
#error  "The USB max FIFO size is 320 x 4 Bytes!"
#endif

/* FUNCTION CONFIGURATION */
#define DEV_MAX_CFG_NUM         (1U)
#define USBD_ITF_MAX_NUM        (2U)
#define USB_MAX_STR_DESC_SIZ    (128U)

#define VBUS_SENSING_ENABLED
//#define SELF_POWER

/* CONFIGURATION FOR HID */
#define HID_IN_PACKET           (64U)
#define HID_OUT_PACKET          (64U)

/* CONFIGURATION FOR CDC */
#define MAX_CDC_PACKET_SIZE     (64U)      /* IN & OUT Endpoint Packet size */
#define CDC_CMD_PACKET_SIZE     (8U)       /* Control Endpoint Packet size */

#define CDC_IN_FRAME_INTERVAL   (5U)       /* Number of frames between IN transfers */
#define APP_RX_DATA_SIZE        (2048U)    /* Total size of IN buffer*/

#ifdef __cplusplus
}
#endif

#endif /* __USB_APP_CONF_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
