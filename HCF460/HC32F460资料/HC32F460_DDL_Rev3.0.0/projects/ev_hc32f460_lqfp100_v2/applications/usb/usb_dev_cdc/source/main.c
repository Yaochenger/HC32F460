/**
 *******************************************************************************
 * @file  usb/usb_dev_cdc/source/main.c
 * @brief Example CDC device.
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
 * @addtogroup USB_Dev_Cdc
 * @{
 */

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
usb_core_instance  usb_dev;

/**
 * @brief  main function
 * @param  None
 * @retval None
 */
int32_t main(void)
{
    usb_dev_init(&usb_dev, &user_desc, &class_cdc_cbk, &user_cb);
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
