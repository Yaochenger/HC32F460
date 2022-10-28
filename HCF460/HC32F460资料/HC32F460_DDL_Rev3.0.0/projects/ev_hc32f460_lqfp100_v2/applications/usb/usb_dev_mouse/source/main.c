/**
 *******************************************************************************
 * @file  usb/usb_dev_mouse/source/main.c
 * @brief Main program of USBD mouse example.
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
 * @addtogroup USB_Dev_Mouse
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
typedef enum {
    BUTTON_NULL = 1U,
    BUTTON_RIGHT,
    BUTTON_LEFT,
    BUTTON_UP,
    BUTTON_DOWN,
} Button_TypeDef;

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define CURSOR_STEP                     (10U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
usb_core_instance usb_dev;

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
 * @brief  Key status read function
 * @param  None
 * @retval button id
 */
static Button_TypeDef Key_ReadIOPin_continuous(void)
{
    Button_TypeDef enKey;
    if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
        enKey = BUTTON_UP;
    } else if (SET == BSP_KEY_GetStatus(BSP_KEY_8)) {
        enKey = BUTTON_DOWN;
    } else if (SET == BSP_KEY_GetStatus(BSP_KEY_4)) {
        enKey = BUTTON_LEFT;
    } else if (SET == BSP_KEY_GetStatus(BSP_KEY_6)) {
        enKey = BUTTON_RIGHT;
    } else {
        enKey = BUTTON_NULL;
    }

    return enKey;
}

/**
 * @brief  get the position of the mouse
 * @param  None
 * @retval Pointer to report
 */
static uint8_t *get_mouse_pos(void)
{
    int8_t  x = (int8_t)0, y = (int8_t)0;
    static uint8_t HID_Buffer [4];

    switch (Key_ReadIOPin_continuous()) {
        case BUTTON_UP:
            y -= (int8_t)CURSOR_STEP;
            break;
        case BUTTON_DOWN:
            y += (int8_t)CURSOR_STEP;
            break;
        case BUTTON_LEFT:
            x -= (int8_t)CURSOR_STEP;
            break;
        case BUTTON_RIGHT:
            x += (int8_t)CURSOR_STEP;
            break;
        default:
            break;
    }
    HID_Buffer[0] = (uint8_t)0;
    HID_Buffer[1] = (uint8_t)x;
    HID_Buffer[2] = (uint8_t)y;
    HID_Buffer[3] = (uint8_t)0;

    return HID_Buffer;
}

/**
 * @brief  SysTick IRQ function that get mouse position and report it
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    uint8_t *buf;

    buf = get_mouse_pos();
    if ((buf[1] != 0U) || (buf[2] != 0U)) {
        usb_dev_mouse_txreport(&usb_dev, buf, 4U);
    }
}

/**
 * @brief  main function for mouse function
 * @param  None
 * @retval int32_t Return value, if needed
 */
int32_t main(void)
{
    usb_dev_init(&usb_dev, &user_desc, &usb_dev_mouse_cbk, &user_cb);
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

