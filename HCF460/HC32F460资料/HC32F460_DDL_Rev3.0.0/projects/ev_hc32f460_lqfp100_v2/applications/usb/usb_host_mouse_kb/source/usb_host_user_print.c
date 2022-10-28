/**
 *******************************************************************************
 * @file  usb/usb_host_mouse_kb/source/usb_host_user_print.c
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

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

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
 * @brief  The function is to handle mouse scroll to upadte the mouse position
 *         through uart port.
 * @param  [in]  x           USB Mouse X co-ordinate
 * @param  [in]  y           USB Mouse Y co-ordinate
 * @retval None
 */
void Mouse_PositionUpdate(int8_t x, int8_t y)
{
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("X: %d, Y: %d\r\n", x, y);
#endif
}

/**
 * @brief  The function is to handle event when the mouse button is pressed
 * @param  [in] button_idx           mouse button pressed
 * @retval None
 */
void Mouse_ButtonPress(uint8_t button_idx)
{
    switch (button_idx) {
        /* Left Button Pressed */
        case 0 :
#if (LL_PRINT_ENABLE == DDL_ON)
            DDL_Printf("L Pressed!\r\n");
#endif
            break;
        /* Right Button Pressed */
        case 1 :
#if (LL_PRINT_ENABLE == DDL_ON)
            DDL_Printf("R Pressed!\r\n");
#endif
            break;
        /* Middle button Pressed */
        case 2 :
#if (LL_PRINT_ENABLE == DDL_ON)
            DDL_Printf("M Pressed!\r\n");
#endif
            break;
        default:
            break;
    }
}

/**
 * @brief  The function is to handle event when the mouse button is released
 * @param  [in] button_idx           mouse button released
 * @retval None
 */
void Mouse_ButtonRelease(uint8_t button_idx)
{
    switch (button_idx) {
        /* Left Button Released */
        case 0 :
#if (LL_PRINT_ENABLE == DDL_ON)
            DDL_Printf("L Released!\r\n");
#endif
            break;
        /* Right Button Released */
        case 1 :
#if (LL_PRINT_ENABLE == DDL_ON)
            DDL_Printf("R Released!\r\n");
#endif
            break;
        /* Middle Button Released */
        case 2 :
#if (LL_PRINT_ENABLE == DDL_ON)
            DDL_Printf("M Released!\r\n");
#endif
            break;
        default:
            break;
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
