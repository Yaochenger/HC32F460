/**
 *******************************************************************************
 * @file  usb/usb_dev_cdc_msc/source/main.c
 * @brief Main file for USB example
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
#include "main.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Cdc_Msc
 * @{
 */

extern usb_dev_class_func usb_dev_composite_cbk;
usb_core_instance  usb_dev;

int32_t main(void)
{
    usb_dev_init(&usb_dev, &user_desc, &usb_dev_composite_cbk, &user_cb);
    for (;;) {
    }
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

