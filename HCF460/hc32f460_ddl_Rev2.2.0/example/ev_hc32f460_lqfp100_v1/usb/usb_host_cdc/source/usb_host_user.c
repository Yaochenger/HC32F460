/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file usb_host_user.c
 **
 ** A detailed description is available at
 ** @link
        This file includes the user application layer.
    @endlink
 **
 **   - 2018-05-21  CDT First version for USB demo.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <string.h>
#include <stdio.h>
#include "usb_host_user.h"
#include "usb_host_cdc_class.h"
#include "usb_host_cdc_ctrl.h"
#include "hc32_ddl.h"


/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* USBH_USR_Private_Macros */
extern usb_core_instance          usb_app_instance;
extern USBH_HOST                  usb_app_host;

extern CDC_Usercb_TypeDef UserCb;

extern CDC_LineCodingTypeDef      CDC_SetLineCode;
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
uint8_t Test_cdc_send_buf[64];
uint8_t cdc_itf_init_fail_cnt = 0;


/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

usb_host_user_callback_func USR_cb =
{
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

/* USBH_USR_Private_Constants */
const char* MSG_HOST_INIT        = "> Host Library Initialized\n";
const char* MSG_DEV_ATTACHED     = "> Device Attached \n";
const char* MSG_DEV_DISCONNECTED = "> Device Disconnected\n";
const char* MSG_DEV_ENUMERATED   = "> Enumeration completed \n";
const char* MSG_DEV_HIGHSPEED    = "> High speed device detected\n";
const char* MSG_DEV_FULLSPEED    = "> Full speed device detected\n";
const char* MSG_DEV_LOWSPEED     = "> Low speed device detected\n";
const char* MSG_DEV_ERROR        = "> Device fault \n";

const char* MSG_DEVICE_CLASS     = "> Class information in the Interface Descriptors\n";
const char* MSG_MSC_CLASS        = "> Mass storage device connected\n";
const char* MSG_HID_CLASS        = "> HID device connected\n";
const char* MSG_CDC_CLASS        = "> CDC device connected\n";
const char* MSG_IF_CDC_CLASS     = "> Interface CDC\n";
const char* MSG_IF_ACM_CLASS     = "> Interface ACM\n";
const char* MSG_VENDOR_CLASS     = "> Device Vendor Specific\n";
const char* MSG_DISK_SIZE        = "> Size of the disk in MBytes: \n";
const char* MSG_LUN              = "> LUN Available in the device:\n";
const char* MSG_ROOT_CONT        = "> Exploring disk flash ...\n";
const char* MSG_WR_PROTECT       = "> The disk is write protected\n";
const char* MSG_UNREC_ERROR      = "> UNRECOVERED ERROR STATE\n";



/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

static void usb_host_cdc_receivedata_Callback(uint8_t * pbuf, uint32_t len);
static void Toggle_Leds(void);
/**
 *******************************************************************************
 ** \brief  Displays the message on LCD for host lib initialization
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_init(void)
{
    static uint8_t startup = 0u;

    if(startup == 0u )
    {
        startup = 1u;

        DDL_Printf(" USB OTG FS CDC Host\n");
        DDL_Printf("> USB Host library started.\n");
        DDL_Printf("     USB Host Library v2.1.0\n" );
    }
}

/**
 *******************************************************************************
 ** \brief  Displays the message on terminal via DDL_Printf
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_devattached(void)
{
    DDL_Printf((void *)MSG_DEV_ATTACHED);
    cdc_itf_init_fail_cnt = 0;
}

/**
 *******************************************************************************
 ** \brief  USBH_USR_UnrecoveredError
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_unrecoverederror(void)
{

  /* Set default screen color*/
    DDL_Printf((void *)MSG_UNREC_ERROR);
}

/**
 *******************************************************************************
 ** \brief  Device disconnect event
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_devdisconn(void)
{
    /* Set default screen color*/
    DDL_Printf((void *)MSG_DEV_DISCONNECTED);
}

/**
 *******************************************************************************
 ** \brief  USBH_USR_ResetUSBDevice
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_devreset(void)
{
    /* callback for USB-Reset */
}

/**
 *******************************************************************************
 ** \brief  USBH_USR_DeviceSpeedDetected
 ** \param  DeviceSpeed : USB speed
 ** \retval None
 ******************************************************************************/
void host_user_devspddetected(uint8_t DeviceSpeed)
{
    if(DeviceSpeed == PRTSPD_FULL_SPEED)
    {
        DDL_Printf((void *)MSG_DEV_FULLSPEED);
    }
    else if(DeviceSpeed == PRTSPD_LOW_SPEED)
    {
        DDL_Printf((void *)MSG_DEV_LOWSPEED);
    }
    else
    {
        DDL_Printf((void *)MSG_DEV_ERROR);
    }
}

/**
 *******************************************************************************
 ** \brief  USBH_USR_Device_DescAvailable
 ** \param  DeviceDesc : device descriptor
 ** \retval None
 ******************************************************************************/
void host_user_devdescavailable(void *DeviceDesc)
{
    usb_host_devdesc_typedef *hs;
    hs = DeviceDesc;

    DDL_Printf("VID : %04lXh\n" , (uint32_t)(*hs).idVendor);
    DDL_Printf("PID : %04lXh\n" , (uint32_t)(*hs).idProduct);
    
}

/**
 *******************************************************************************
 ** \brief  USBH_USR_DeviceAddressAssigned
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_devaddrdistributed(void)
{

}

/**
 *******************************************************************************
 ** \brief  USBH_USR_Configuration_DescAvailable
 ** \param  cfgDesc : Configuration desctriptor
 ** \param  itfDesc : Interface desctriptor
 ** \param  epDesc : Endpoint desctriptor
 ** \retval None
 ******************************************************************************/
void host_user_cfgdescavailable(usb_host_cfgdesc_typedef * cfgDesc,
                                usb_host_itfdesc_typedef *itfDesc,
                                USB_HOST_EPDesc_TypeDef *epDesc)
{
    usb_host_itfdesc_typedef *id;

    id = itfDesc;

    if((*id).bInterfaceClass  == 0x08u)
    {
        DDL_Printf((void *)MSG_MSC_CLASS);
    }
    else if((*id).bInterfaceClass  == 0x03u)
    {
        DDL_Printf((void *)MSG_HID_CLASS);
    }
    else if((*id).bInterfaceClass  == 0x02u)
    {
        DDL_Printf((void *)MSG_IF_ACM_CLASS);
    }
    else if((*id).bInterfaceClass  == 0x0Au)
    {
        DDL_Printf((void *)MSG_IF_CDC_CLASS);
    }
    else if((*id).bInterfaceClass  == 0xFFu)
    {
        DDL_Printf((void *)MSG_VENDOR_CLASS);
    }
}

/**
 *******************************************************************************
 ** \brief  Displays the message on LCD for Manufacturer String
 ** \param  ManufacturerString
 ** \retval None
 ******************************************************************************/
void host_user_mfcstring(void *ManufacturerString)
{
    DDL_Printf("Manufacturer : %s\n", (char *)ManufacturerString);
}

/**
 *******************************************************************************
 ** \brief  Displays the message on LCD for product String
 ** \param  ProductString
 ** \retval None
 ******************************************************************************/
void host_user_productstring(void *ProductString)
{
    DDL_Printf("Product : %s\n", (char *)ProductString);
}

/**
 *******************************************************************************
 ** \brief  Displays the message on LCD for SerialNum_String
 ** \param  SerialNumString
 ** \retval None
 ******************************************************************************/
void host_user_serialnum(void *SerialNumString)
{
    DDL_Printf( "Serial Number : %s\n", (char *)SerialNumString);
}

/**
 *******************************************************************************
 ** \brief  User response request is displayed to ask application jump to class
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_enumcompl(void)
{

    /* Enumeration complete */
    DDL_Printf((void *)MSG_DEV_ENUMERATED);

    UserCb.Receive = usb_host_cdc_receivedata_Callback;
}

/**
 *******************************************************************************
 ** \brief  Device is not supported
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_devunsupported(void)
{
    cdc_itf_init_fail_cnt++;
    if(1 == cdc_itf_init_fail_cnt)
    {
        DDL_Printf ("> Device is not a standard CDC ACM device.\n");
    }
    else if(2 == cdc_itf_init_fail_cnt)
    {
        DDL_Printf ("> Device is not a standard CDC ACM device.\n");
        DDL_Printf ("> Try to Parse Vendor CDC device.\n");
    }
    else if(3 == cdc_itf_init_fail_cnt)
    {
        DDL_Printf ("> Device is not supported.\n");
    }
    
}

/**
 *******************************************************************************
 ** \brief  User Action for application state entry
 ** \param  None
 ** \retval HOST_USER_STATUS : User response for key button
 ******************************************************************************/
HOST_USER_STATUS host_user_userinput(void)
{
    HOST_USER_STATUS usbh_usr_status;

    usbh_usr_status = USER_NONE_RESP;

    /*Key is in polling mode to detect user action */
    if(BSP_KEY_GetStatus(BSP_KEY_2) == Reset)
    {
        usbh_usr_status = USER_HAVE_RESP;
    }

    return usbh_usr_status;
}

/**
 *******************************************************************************
 ** \brief  Over Current Detected on VBUS
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_overcurrent(void)
{
    DDL_Printf ("Overcurrent detected.\n");
}
/**
* @brief  CDC receive data callback.
* @param  ptr: pointer to data received.
* @retval None
*/
static void usb_host_cdc_receivedata_Callback(uint8_t * pbuf, uint32_t len)
{
    DDL_Printf("CDC Receive %4d data:", len);
    DDL_Printf((void *)pbuf);
}
/**
 *******************************************************************************
 ** \brief  Demo application for cdc
 ** \param  None
 ** \retval None
 ******************************************************************************/
int host_user_cdc_app(void)
{
    static uint8_t en_cdc_receive = 0;
    static uint32_t sendcnt = 0;
    
    Toggle_Leds();
    
    if(!en_cdc_receive)
    {
        en_cdc_receive = 1;
        usb_host_cdc_enable_receive(&usb_app_instance);
        DDL_Printf ("Press SW2 to Test Send Data\n");
        DDL_Printf ("Press SW3 to Change VCP Buadrate\n");
    }
    /*Key is in polling mode to detect user action */
    if(BSP_KEY_GetStatus(BSP_KEY_2) == Set)
    {
        sendcnt ++;
        DDL_Printf ("CDC Test Send Data Cnt:%d\n",sendcnt);
        sprintf((char*)Test_cdc_send_buf,"HDSC CDC Send Data Cnt:%d\n",sendcnt);
        usb_host_cdc_senddata((uint8_t*)Test_cdc_send_buf,strlen((const char*)Test_cdc_send_buf));
    }
    if(BSP_KEY_GetStatus(BSP_KEY_3) == Set)
    {
        memset(&CDC_SetLineCode,0,sizeof(CDC_SetLineCode));
        CDC_SetLineCode.b.dwDTERate = 115200;
        CDC_SetLineCode.b.bDataBits = 8;//8 bit data
        CDC_SetLineCode.b.bCharFormat = 0;//1 stop bit
        CDC_SetLineCode.b.bParityType = 0;//none
        DDL_Printf ("Change CDC VCP Buadrate to %d\n",CDC_SetLineCode.b.dwDTERate);
        usb_host_cdc_issue_setlinecoding(&usb_app_instance,&usb_app_host);
    }
    return((int)0);
}
/**
 *******************************************************************************
 ** \brief  Toggle leds to shows user input state
 ** \param  None
 ** \retval None
 ******************************************************************************/
static void Toggle_Leds(void)
{
    static uint32_t i;
    if (i++ == 0x10000u)
    {
        BSP_LED_Toggle(LED_RED);
        BSP_LED_Toggle(LED_GREEN);
        i = 0u;
    }
}
/**
 *******************************************************************************
 ** \brief  Deint User state and associated variables
 ** \param  None
 ** \retval None
 ******************************************************************************/
void host_user_denint(void)
{
    //uset state deinit
}


/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
