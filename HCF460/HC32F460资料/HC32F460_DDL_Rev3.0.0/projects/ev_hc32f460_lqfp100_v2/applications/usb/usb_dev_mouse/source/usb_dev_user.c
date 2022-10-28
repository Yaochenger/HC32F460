/**
 *******************************************************************************
 * @file  usb/usb_dev_mouse/source/usb_dev_user.c
 * @brief User callback function for USB example
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
#include "usb_dev_user.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Mouse
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
void usb_dev_user_init(void);
void usb_dev_user_rst(void);
void usb_dev_user_devcfg(void);
void usb_dev_user_devsusp(void);
void usb_dev_user_devresume(void);
void usb_dev_user_conn(void);
void usb_dev_user_disconn(void);

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
usb_dev_user_func user_cb = {
    &usb_dev_user_init,
    &usb_dev_user_rst,
    &usb_dev_user_devcfg,
    &usb_dev_user_devsusp,
    &usb_dev_user_devresume,
    &usb_dev_user_conn,
    &usb_dev_user_disconn,
};

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
 * @brief  usb_dev_user_init
 * @param  None
 * @retval None
 */
void usb_dev_user_init(void)
{
    /* Key initialize */
    BSP_KEY_Init();
    /* Setup SysTick Timer for 20 msec interrupts This interrupt is used to probe the joystick */
    if (0U != SysTick_Config(SystemCoreClock / 50U)) {
        /* Capture error */
        for (;;) {
            ;
        }
    }
}

/**
 * @brief  usb_dev_user_rst
 * @param  None
 * @retval None
 */
void usb_dev_user_rst(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(">>USB Device has reseted.\r\n");
#endif
}

/**
 * @brief  usb_dev_user_devcfg
 * @param  None
 * @retval None
 */
void usb_dev_user_devcfg(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(">>MOUSE HID interface starts.\r\n");
#endif
}

/**
 * @brief  usb_dev_user_conn
 * @param  None
 * @retval None
 */
void usb_dev_user_conn(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(">>USB device connects.\r\n");
#endif
}

/**
 * @brief  USBD_USR_DeviceDisonnected
 * @param  None
 * @retval None
 */
void usb_dev_user_disconn(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(">>USB device disconnected.\r\n");
#endif
}

/**
 * @brief  usb_dev_user_devsusp
 * @param  None
 * @retval None
 */
void usb_dev_user_devsusp(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(">>USB device in suspend status.\r\n");
#endif
}

/**
 * @brief  usb_dev_user_devresume
 * @param  None
 * @retval None
 */
void usb_dev_user_devresume(void)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf(">>USB device resumes.\r\n");
#endif
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
