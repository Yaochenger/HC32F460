/**
 *******************************************************************************
 * @file  usb/usb_host_mouse_kb/source/usb_host_user_print.h
 * @brief Header file for usb_host_user_print.c
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
#ifndef __USB_HOST_USER_PRINT_H__
#define __USB_HOST_USER_PRINT_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "usb_host_hid_class.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Host_Mouse_kb
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void Mouse_PositionUpdate(int8_t x, int8_t y);
extern void Mouse_ButtonRelease(uint8_t button_idx);
extern void Mouse_ButtonPress(uint8_t button_idx);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __USB_HOST_USER_PRINT_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
