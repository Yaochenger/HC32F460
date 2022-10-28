/**
 *******************************************************************************
 * @file  usb/usb_host_msc/source/usb_host_user.h
 * @brief Header file for usb_host_user.c
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
#ifndef __USB_HOST_USER_H__
#define __USB_HOST_USER_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
//#include "ff.h"
#include "usb_host_core.h"
#include "usb_app_conf.h"
#include <stdio.h>
#include "usb_host_msc_class.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Host_Msc
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* State Machine for the USBH_USR_ApplicationState */
#define USH_USR_FS_INIT                 (0U)
#define USH_USR_FS_READLIST             (1U)
#define USH_USR_FS_WRITEFILE            (2U)
#define USH_USR_FS_IDLE                 (3U)

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/
extern usb_host_user_callback_func USR_cb;
extern uint8_t USB_HOST_USER_AppState;

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void host_user_init(void);
extern void host_user_denint(void);
extern void host_user_devattached(void);
extern void host_user_devreset(void);
extern void host_user_devdisconn(void);
extern void host_user_overcurrent(void);
extern void host_user_devspddetected(uint8_t DeviceSpeed);
extern void host_user_devdescavailable(void *DeviceDesc);
extern void host_user_devaddrdistributed(void);
extern void host_user_cfgdescavailable(usb_host_cfgdesc_typedef *cfgDesc,
                                       usb_host_itfdesc_typedef *itfDesc,
                                       USB_HOST_EPDesc_TypeDef *epDesc);
extern void host_user_mfcstring(void *ManufacturerString);
extern void host_user_productstring(void *ProductString);
extern void host_user_serialnum(void *SerialNumString);
extern void host_user_enumcompl(void);
extern HOST_USER_STATUS host_user_userinput(void);
extern void host_user_devunsupported(void);
extern void host_user_unrecoverederror(void);
extern int host_user_msc_app(void);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /*__USB_HOST_USER_H__*/

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


