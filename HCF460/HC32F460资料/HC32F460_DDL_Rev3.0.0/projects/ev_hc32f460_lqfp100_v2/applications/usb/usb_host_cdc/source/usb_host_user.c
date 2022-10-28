/**
 *******************************************************************************
 * @file  usb/usb_host_cdc/source/usb_host_user.c
 * @brief user application layer.
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
#include <string.h>
#include <stdio.h>
#include "usb_host_user.h"
#include "usb_host_cdc_class.h"
#include "usb_host_cdc_ctrl.h"
#include "usb_host_driver.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Host_Cdc
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
extern usb_core_instance          usb_app_instance;
extern USBH_HOST                  usb_app_host;

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
usb_host_user_callback_func USR_cb = {
    &host_user_init,
    &host_user_denint,
    &host_user_devattached,
    &host_user_devreset,
    &host_user_devdisconn,
    &host_user_overcurrent,
    &host_user_devspddetected,
    &host_user_devdescavailable,
    &host_user_devaddrdistributed,
    &host_user_cfgdescavailable,
    &host_user_mfcstring,
    &host_user_productstring,
    &host_user_serialnum,
    &host_user_enumcompl,
    &host_user_userinput,
    &host_user_cdc_app,
    &host_user_devunsupported,
    &host_user_unrecoverederror

};

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void usb_host_cdc_receivedata_Callback(uint8_t *pbuf, uint32_t len);
static void Toggle_Leds(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
/* USBH_USR_Private_Constants */
const static char *MSG_DEV_ATTACHED     = "> Device Attached\r\n";
const static char *MSG_DEV_DISCONNECTED = "> Device Disconnected\r\n";
const static char *MSG_DEV_ENUMERATED   = "> Enumeration completed\r\n";
const static char *MSG_DEV_FULLSPEED    = "> Full speed device detected\r\n";
const static char *MSG_DEV_LOWSPEED     = "> Low speed device detected\r\n";
const static char *MSG_DEV_ERROR        = "> Device fault\r\n";

const static char *MSG_MSC_CLASS        = "> Mass storage device connected\r\n";
const static char *MSG_IF_CDC_CLASS     = "> Interface CDC\n";
const static char *MSG_IF_ACM_CLASS     = "> Interface ACM\n";
const static char *MSG_VENDOR_CLASS     = "> Device Vendor Specific\n";
const static char *MSG_HID_CLASS        = "> HID device connected\r\n";
const static char *MSG_UNREC_ERROR      = "> UNRECOVERED ERROR STATE\r\n";

static uint8_t cdc_itf_init_fail_cnt = 0U;
static uint8_t en_cdc_receive = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Displays the message on terminal for host lib initialization
 * @param  None
 * @retval None
 */
void host_user_init(void)
{
    static uint8_t startup = 0U;

    if (startup == 0U) {
        startup = 1U;
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf("> USB Host library started.\r\n");
        DDL_Printf("     USB Host Library v2.1.0\r\n");
#endif
    }
}

/**
 * @brief  Displays the message on terminal via DDL_Printf
 * @param  None
 * @retval None
 */
void host_user_devattached(void)
{
    cdc_itf_init_fail_cnt = 0U;
    en_cdc_receive = 0U;
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(MSG_DEV_ATTACHED);
#endif
}

/**
 * @brief  host_user_unrecoverederror
 * @param  None
 * @retval None
 */
void host_user_unrecoverederror(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(MSG_UNREC_ERROR);
#endif
}

/**
 * @brief  Device disconnect event
 * @param  None
 * @retval None
 */
void host_user_devdisconn(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(MSG_DEV_DISCONNECTED);
#endif
}

/**
 * @brief  USBH_USR_ResetUSBDevice
 * @param  None
 * @retval None
 */
void host_user_devreset(void)
{
    /* callback for USB-Reset */
}

/**
 * @brief  host_user_devspddetected
 * @param  [in] DeviceSpeed      USB speed
 * @retval None
 */
void host_user_devspddetected(uint8_t DeviceSpeed)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    if (DeviceSpeed == PRTSPD_FULL_SPEED) {
        DDL_Printf(MSG_DEV_FULLSPEED);
    } else if (DeviceSpeed == PRTSPD_LOW_SPEED) {
        DDL_Printf(MSG_DEV_LOWSPEED);
    } else {
        DDL_Printf(MSG_DEV_ERROR);
    }
#endif
}

/**
 * @brief  host_user_devdescavailable
 * @param  [in] DeviceDesc       device descriptor
 * @retval None
 */
void host_user_devdescavailable(void *DeviceDesc)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    usb_host_devdesc_typedef *hs;
    hs = DeviceDesc;
    DDL_Printf("VID : %04lXh\r\n", (uint32_t)(*hs).idVendor);
    DDL_Printf("PID : %04lXh\r\n", (uint32_t)(*hs).idProduct);
#endif
}

/**
 * @brief  host_user_devaddrdistributed
 * @param  None
 * @retval None
 */
void host_user_devaddrdistributed(void)
{
}

/**
 * @brief  host_user_cfgdescavailable
 * @param  [in] cfgDesc          Configuration desctriptor
 * @param  [in] itfDesc          Interface desctriptor
 * @param  [in] epDesc           Endpoint desctriptor
 * @retval None
 */
void host_user_cfgdescavailable(usb_host_cfgdesc_typedef *cfgDesc,
                                usb_host_itfdesc_typedef *itfDesc,
                                USB_HOST_EPDesc_TypeDef *epDesc)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    usb_host_itfdesc_typedef *id;

    id = itfDesc;
    if ((*id).bInterfaceClass  == 0x08U) {
        DDL_Printf(MSG_MSC_CLASS);
    } else if ((*id).bInterfaceClass  == 0x03U) {
        DDL_Printf(MSG_HID_CLASS);
    } else if ((*id).bInterfaceClass  == 0x02U) {
        DDL_Printf(MSG_IF_ACM_CLASS);
    } else if ((*id).bInterfaceClass  == 0x0AU) {
        DDL_Printf(MSG_IF_CDC_CLASS);
    } else if ((*id).bInterfaceClass  == 0xFFU) {
        DDL_Printf(MSG_VENDOR_CLASS);
    } else {
        ;
    }
#endif
}

/**
 * @brief  Displays the message on terminal for Manufacturer String
 * @param  [in] ManufacturerString
 * @retval None
 */
void host_user_mfcstring(void *ManufacturerString)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("Manufacturer : %s\r\n", (char *)ManufacturerString);
#endif
}

/**
 * @brief  Displays the message on terminal for product String
 * @param  [in] ProductString
 * @retval None
 */
void host_user_productstring(void *ProductString)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("Product : %s\r\n", (char *)ProductString);
#endif
}

/**
 * @brief  Displays the message on terminal for SerialNum_String
 * @param  [in] SerialNumString
 * @retval None
 */
void host_user_serialnum(void *SerialNumString)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("Serial Number : %s\r\n", (char *)SerialNumString);
#endif
}

/**
 * @brief  User response request is displayed to ask application jump to class
 * @param  None
 * @retval None
 */
void host_user_enumcompl(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    /* Enumeration complete */
    DDL_Printf(MSG_DEV_ENUMERATED);
#endif
    UserCb.Receive = usb_host_cdc_receivedata_Callback;
}

/**
 * @brief  Device is not supported
 * @param  None
 * @retval None
 */
void host_user_devunsupported(void)
{
    cdc_itf_init_fail_cnt++;
#if (LL_PRINT_ENABLE == DDL_ON)
    if (1U == cdc_itf_init_fail_cnt) {
        DDL_Printf("> Device is not a standard CDC ACM device.\n");
    } else if (2U == cdc_itf_init_fail_cnt) {
        DDL_Printf("> Device is not a standard CDC ACM device.\n");
        DDL_Printf("> Try to Parse Vendor CDC device.\n");
    } else if (3U == cdc_itf_init_fail_cnt) {
        DDL_Printf("> Device is not supported.\n");
    } else {
        ;
    }
#endif
}

/**
 * @brief  User Action for application state entry
 * @param  None
 * @retval HOST_USER_STATUS     User response for key button
 */
HOST_USER_STATUS host_user_userinput(void)
{
    HOST_USER_STATUS usbh_usr_status;

    usbh_usr_status = USER_NONE_RESP;
    /* Key is in polling mode to detect user action */
    if (BSP_KEY_GetStatus(BSP_KEY_2) == SET) {
        usbh_usr_status = USER_HAVE_RESP;
    }

    return usbh_usr_status;
}

/**
 * @brief  Over Current Detected on VBUS
 * @param  None
 * @retval None
 */
void host_user_overcurrent(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("Overcurrent detected.\r\n");
#endif
}

/**
 * @brief  CDC receive data callback.
 * @param  [in] pbuf    pointer to data received.
 * @param  [in] len     data received len.
 * @retval None
 */
static void usb_host_cdc_receivedata_Callback(uint8_t *pbuf, uint32_t len)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("> cdc data in\r\n");
#endif
    /* send back received data */
    usb_host_cdc_senddata(pbuf, (uint16_t)len);
}

/**
 * @brief  Demo application for cdc
 * @param  None
 * @retval None
 */
int host_user_cdc_app(void)
{
    Toggle_Leds();

    if (0U == en_cdc_receive) {
        en_cdc_receive = 1U;

        /* line config */
        (void)memset(&CDC_SetLineCode, 0, sizeof(CDC_SetLineCode));
        CDC_SetLineCode.b.dwDTERate = 115200;
        CDC_SetLineCode.b.bDataBits = 8;//8 bit data
        CDC_SetLineCode.b.bCharFormat = 0;//1 stop bit
        CDC_SetLineCode.b.bParityType = 0;//none
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf("Set vcp linecfg: 115200bps, 8N1\r\n");
#endif
        usb_host_cdc_issue_setlinecoding(&usb_app_instance, &usb_app_host);

        usb_host_cdc_enable_receive(&usb_app_instance);
    }

    return ((int)0);
}

/**
 * @brief  Toggle leds to shows user input state
 * @param  None
 * @retval None
 */
static void Toggle_Leds(void)
{
    static uint32_t i;
    if (i++ == 0x10000U) {
        BSP_LED_Toggle(LED_BLUE);
        BSP_LED_Toggle(LED_YELLOW);
        i = 0U;
    }
}

/**
 * @brief  Deint User state and associated variables
 * @param  None
 * @retval None
 */
void host_user_denint(void)
{
    /* uset state deinit */
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
