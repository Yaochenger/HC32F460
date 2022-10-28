/**
 *******************************************************************************
 * @file  usb/usb_host_mouse_kb/source/usb_host_user.c
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
#include "usb_host_user.h"
#include "usb_host_hid_mouseapp.h"
#include "usb_host_hid_keyboardapp.h"
#include "usb_host_user_print.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Host_Mouse_kb
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define BUTTON_PORT         (PortD)
#define BUTTON_PIN          (Pin03)

#define GET_BUTTON_KEY()    (PORT_GetBit(PortD, Pin03))

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
usb_host_user_callback_func USER_cbk = {
    host_user_init,
    host_user_denint,
    host_user_devattached,
    host_user_devreset,
    host_user_devdisconn,
    host_user_overcurrent,
    host_user_devspddetected,
    host_user_devdescavailable,
    host_user_devaddrdistributed,
    host_user_cfgdescavailable,
    host_user_mfcstring,
    host_user_productstring,
    host_user_serialnum,
    host_user_enumcompl,
    host_user_userinput,
    NULL,
    host_user_devunsupported,
    host_user_unrecoverederror
};

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
/*--------------- Messages ---------------*/
const static char *MSG_DEV_ATTACHED       = ">>Device Attached\r\n";
const static char *MSG_DEV_DISCONNECTED   = ">>Device Disconnected\r\n";
const static char *MSG_DEV_ENUMERATED     = ">>Enumeration completed\r\n";
const static char *MSG_DEV_FULLSPEED      = ">>Full speed device detected\r\n";
const static char *MSG_DEV_LOWSPEED       = ">>Low speed device detected\r\n";
const static char *MSG_DEV_ERROR          = ">>Device fault\r\n";

const static char *MSG_MSC_CLASS          = ">>Mass storage device connected\r\n";
const static char *MSG_HID_CLASS          = ">>HID device connected\r\n";

const static char *USB_HID_KeybrdStatus   = ">>HID Demo Device : Keyboard\r\n";
const static char *MSG_UNREC_ERROR        = ">>UNRECOVERED ERROR STATE\r\n";

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Displays the message on debugging terminal for host lib initialization
 * @param  None
 * @retval None
 */
void host_user_init(void)
{
    static uint8_t startup = 0;

    if (startup == 0U) {
        startup = 1;
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf("> USB Host library started.\r\n");
#endif
    }
}

/**
 * @brief  Displays the message on debugging terminal as device attached
 * @param  None
 * @retval None
 */
void host_user_devattached(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(MSG_DEV_ATTACHED);
#endif
}

/**
 * @brief
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
 * @brief  device disconnect event
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
 * @brief  device reset
 * @param  None
 * @retval None
 */
void host_user_devreset(void)
{
    /* Users can do their application actions here for the USB-Reset */
}

/**
 * @brief  Displays the message on terminal for device speed
 * @param  [in] DeviceSpeed      device speed
 * @retval None
 */
void host_user_devspddetected(uint8_t DeviceSpeed)
{
    if (DeviceSpeed == PRTSPD_FULL_SPEED) {
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf(MSG_DEV_FULLSPEED);
#endif
    } else if (DeviceSpeed == PRTSPD_LOW_SPEED) {
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf(MSG_DEV_LOWSPEED);
#endif
    } else {
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf(MSG_DEV_ERROR);
#endif
    }
}

/**
 * @brief  Displays the message on terminal for device descriptor
 * @param  [in] DeviceDesc           device descriptor
 * @retval None
 */
void host_user_devdescavailable(void *DeviceDesc)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    uint8_t temp[50];
    usb_host_devdesc_typedef *hs;

    hs = DeviceDesc;
    (void)sprintf((char *)temp, "VID : %04lXh\r\n", (uint32_t)(*hs).idVendor);
    DDL_Printf((void *)temp);
    (void)sprintf((char *)temp, "PID : %04lXh\r\n", (uint32_t)(*hs).idProduct);
    DDL_Printf((void *)temp);
#endif
}

/**
 * @brief  USB device is successfully distributed a Address
 * @param  None
 * @retval None
 */
void host_user_devaddrdistributed(void)
{
}

/**
 * @brief  Displays the message on terminal for configuration descriptor
 * @param  [in] cfgDesc          configuration descriptor
 * @param  [in] itfDesc          interface descriptor
 * @param  [in] epDesc           EP descriptor
 * @retval None
 */
void host_user_cfgdescavailable(usb_host_cfgdesc_typedef *cfgDesc,
                                usb_host_itfdesc_typedef *itfDesc,
                                USB_HOST_EPDesc_TypeDef *epDesc)
{
    usb_host_itfdesc_typedef *id;

    id = itfDesc;
    if ((*id).bInterfaceClass  == 0x08U) {
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf("%s", MSG_MSC_CLASS);
#endif
    } else if ((*id).bInterfaceClass  == 0x03U) {
#if (LL_PRINT_ENABLE == DDL_ON)
        DDL_Printf("%s", MSG_HID_CLASS);
#endif
    } else {
        /*reserved*/
    }
}

/**
 * @brief  Displays the Manufacturer String
 * @param  [in] ManufacturerString       MFC string
 * @retval None
 */
void host_user_mfcstring(void *ManufacturerString)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    char temp[100];
    (void)sprintf(temp, "Manufacturer : %s\r\n", (char *)ManufacturerString);
    DDL_Printf((void *)temp);
#endif
}

/**
 * @brief  display product string
 * @param  [in] ProductString        product string
 * @retval None
 */
void host_user_productstring(void *ProductString)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    char temp[100];
    (void)sprintf((char *)temp, "Product : %s\r\n", (char *)ProductString);
    DDL_Printf((void *)temp);
#endif
}

/**
 * @brief  display the serial number string
 * @param  [in] SerialNumString      serial number string pointer
 * @retval None
 */
void host_user_serialnum(void *SerialNumString)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    uint8_t temp[100];
    (void)sprintf((char *)temp, "Serial Number : %s\r\n", (char *)SerialNumString);
    DDL_Printf((void *)temp);
#endif
}

/**
 * @brief  display enumeration complete
 * @param  None
 * @retval None
 */
void host_user_enumcompl(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    /* Enumeration complete */
    DDL_Printf(MSG_DEV_ENUMERATED);
#endif
}

/**
 * @brief  display that host does not support the device connected
 * @param  None
 * @retval None
 */
void host_user_devunsupported(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("> Device not supported.\r\n");
#endif
}

/**
 * @brief  user input
 * @param  None
 * @retval None
 */
HOST_USER_STATUS host_user_userinput(void)
{
    HOST_USER_STATUS usbh_usr_status;

    usbh_usr_status = USER_NONE_RESP;
    return usbh_usr_status;
}

/**
 * @brief  display that have detected device over current
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
 * @brief  initialize mouse window
 * @param  None
 * @retval None
 */
void user_mouse_init(void)
{
    Mouse_ButtonRelease(0);
    Mouse_ButtonRelease(1);
    Mouse_ButtonRelease(2);
}

/**
 * @brief  process the mouse data
 * @param  [in] data         mouse data would be decode
 * @retval None
 */
void user_mouse_dataprocess(HID_MOUSE_Data_TypeDef *data)
{

    uint8_t idx;
    static uint8_t b_state[3] = { 0U, 0U, 0U};

    if ((data->x != 0U) && (data->y != 0U)) {
        Mouse_PositionUpdate((int8_t)data->x, (int8_t)data->y);
    }
    for (idx = 0U ; idx < 3U ; idx ++) {
        if (0U != (data->button & (0x01U << idx))) {
            if (b_state[idx] == 0U) {
                Mouse_ButtonPress(idx);
                b_state[idx] = 1U;
            }
        } else {
            if (b_state[idx] == 1U) {
                Mouse_ButtonRelease(idx);
                b_state[idx] = 0U;
            }
        }
    }
}

/**
 * @brief  initialize the key board
 * @param  None
 * @retval None
 */
void user_keyboard_init(void)
{
    DDL_Printf(USB_HID_KeybrdStatus);
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("> Use Keyboard to tape characters: \n\n");
    DDL_Printf("\n\n\n\n\n\n");
#endif
}

/**
 * @brief  display key value
 * @param  [in] data         key value
 * @retval None
 */
void user_keyboard_dataprocess(uint8_t data)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("%c", (char)data);
#endif
}

/**
 * @brief  deint user state and other related state if needed
 * @param  None
 * @retval None
 */
void host_user_denint(void)
{
}

/**
 * @}
 */

/**
 * @}
 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
