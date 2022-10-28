/**
 *******************************************************************************
 * @file  usart/usart_uart_int/source/ring_buf.c
 * @brief This file provides firmware functions to manage the ring buffer.
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
#include "ring_buf.h"
#include "hc32_ll_utility.h"

/**
 * @addtogroup USART_UART_Interrupt
 * @{
 */

/**
 * @defgroup Ring_Buffer Ring Buffer
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
 * @defgroup Ring_Buffer_Global_Functions Ring Buffer Global Functions
 * @{
 */

/**
 * @brief  Initialize ring buffer.
 * @param  [in] pstcBuf                 Pointer to a @ref stc_ring_buf_t structure
 * @param  [in] pu8Data                 Data buffer
 * @param  [in] u32Size                 Data buffer size
 * @retval int32_t:
 *           - LL_OK:                   Initialize successfully.
 *           - LL_ERR_INVD_PARAM:       If one of following cases matches:
 *                                      - The pointer pstcBuf value is NULL.
 *                                      - The pointer pu8Data value is NULL.
 *                                      - The u32Size value is 0.
 */
int32_t BUF_Init(stc_ring_buf_t *pstcBuf, uint8_t *pu8Data, uint32_t u32Size)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((pstcBuf != NULL) && (pu8Data != NULL) && (u32Size != 0UL)) {
        pstcBuf->pu8Data = pu8Data;
        pstcBuf->u32In = 0;
        pstcBuf->u32Out = 0;
        pstcBuf->u32Size = u32Size;
        pstcBuf->u32FreeSize = u32Size;
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Ring buffer free size.
 * @param  [in] pstcBuf                 Pointer to a @ref stc_ring_buf_t structure
 * @retval Ring buffer free size
 */
uint32_t BUF_FreeSize(const stc_ring_buf_t *pstcBuf)
{
    return pstcBuf->u32FreeSize;
}

/**
 * @brief  Ring buffer used size.
 * @param  [in] pstcBuf                 Pointer to a @ref stc_ring_buf_t structure
 * @retval Ring buffer used size
 */
uint32_t BUF_UsedSize(const stc_ring_buf_t *pstcBuf)
{
    return (pstcBuf->u32Size - pstcBuf->u32FreeSize);
}

/**
 * @brief  Ring buffer full.
 * @param  [in] pstcBuf                 Pointer to a @ref stc_ring_buf_t structure
 * @retval Ring buffer status
 */
bool BUF_Full(const stc_ring_buf_t *pstcBuf)
{
    return (0UL == pstcBuf->u32FreeSize);
}

/**
 * @brief  Check ring buffer empty.
 * @param  [in] pstcBuf                 Pointer to a @ref stc_ring_buf_t structure
 * @retval Ring buffer status
 */
bool BUF_Empty(const stc_ring_buf_t *pstcBuf)
{
    return (pstcBuf->u32Size == pstcBuf->u32FreeSize);
}

/**
 * @brief  Read ring buffer.
 * @param  [in] pstcBuf                 Pointer to a @ref stc_ring_buf_t structure
 * @param  [in] au8Data                 Pointer to data buffer to read
 * @param  [in] u32Len                  Data length
 * @retval Read length
 */
uint32_t BUF_Read(stc_ring_buf_t *pstcBuf, uint8_t au8Data[], uint32_t u32Len)
{
    uint32_t u32CopyLen;
    uint32_t u32ReadLen = 0UL;

    if ((pstcBuf != NULL) && (au8Data != NULL) && (u32Len != 0UL)) {
        u32ReadLen = BUF_UsedSize(pstcBuf);
        if (u32ReadLen >= u32Len) {
            u32ReadLen = u32Len;
        }

        if (pstcBuf->u32Out + u32ReadLen <= pstcBuf->u32Size) {
            (void)memcpy(au8Data, &pstcBuf->pu8Data[pstcBuf->u32Out], u32ReadLen);
        } else {
            u32CopyLen = pstcBuf->u32Size - pstcBuf->u32Out;
            (void)memcpy(&au8Data[0], &pstcBuf->pu8Data[pstcBuf->u32Out], u32CopyLen);
            (void)memcpy(&au8Data[u32CopyLen], &pstcBuf->pu8Data[0], u32ReadLen - u32CopyLen);
        }

        __disable_irq();
        pstcBuf->u32FreeSize += u32ReadLen;
        pstcBuf->u32Out += u32ReadLen;
        if (pstcBuf->u32Out >= pstcBuf->u32Size) {
            pstcBuf->u32Out %= pstcBuf->u32Size;
        }
        __enable_irq();
    }

    return u32ReadLen;
}

/**
 * @brief  Write ring buffer.
 * @param  [in] pstcBuf                 Pointer to a @ref stc_ring_buf_t structure
 * @param  [in] au8Data                 Pointer to data buffer to write
 * @param  [in] u32Len                  Data length
 * @retval Write length
 */
uint32_t BUF_Write(stc_ring_buf_t *pstcBuf, uint8_t au8Data[], uint32_t u32Len)
{
    uint32_t u32CopyLen;
    uint32_t u32WriteLen = 0UL;

    if ((pstcBuf != NULL) && (au8Data != NULL) && (u32Len != 0UL)) {
        u32WriteLen = BUF_FreeSize(pstcBuf);

        if (u32Len <= u32WriteLen) {
            u32WriteLen = u32Len;
        }

        if (pstcBuf->u32In + u32WriteLen <= pstcBuf->u32Size) {
            (void)memcpy(&pstcBuf->pu8Data[pstcBuf->u32In], au8Data, u32WriteLen);
        } else {
            u32CopyLen = pstcBuf->u32Size - pstcBuf->u32In;
            (void)memcpy(&pstcBuf->pu8Data[pstcBuf->u32In], &au8Data[0], u32CopyLen);
            (void)memcpy(&pstcBuf->pu8Data[0], &au8Data[u32CopyLen], u32WriteLen - u32CopyLen);
        }

        __disable_irq();
        pstcBuf->u32FreeSize -= u32WriteLen;
        pstcBuf->u32In += u32WriteLen;
        if (pstcBuf->u32In >= pstcBuf->u32Size) {
            pstcBuf->u32In %= pstcBuf->u32Size;
        }
        __enable_irq();
    }

    return u32WriteLen;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
