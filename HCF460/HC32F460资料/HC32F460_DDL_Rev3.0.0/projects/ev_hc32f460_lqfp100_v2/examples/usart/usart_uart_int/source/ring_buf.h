/**
 *******************************************************************************
 * @file  usart/usart_uart_int/source/ring_buf.h
 * @brief This file contains all the functions prototypes of the ring buffer.
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

#ifndef __RING_BUF_H__
#define __RING_BUF_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdbool.h>
#include "hc32_ll_def.h"

/**
 * @addtogroup USART_UART_Interrupt
 * @{
 */

/**
 * @addtogroup Ring_Buffer
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 * @defgroup Ring_Buffer_Global_Types Ring Buffer Global Types
 * @{
 */

/**
 * @brief Ring buffer structure definition
 */
typedef struct {
    uint8_t  *pu8Data;
    uint32_t u32In;
    uint32_t u32Out;
    uint32_t u32Size;
    uint32_t u32FreeSize;
} stc_ring_buf_t;

/**
 * @}
 */

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/

/**
 * @addtogroup Ring_Buffer_Global_Functions
 * @{
 */

int32_t BUF_Init(stc_ring_buf_t *pstcBuf, uint8_t *pu8Data, uint32_t u32Size);

uint32_t BUF_FreeSize(const stc_ring_buf_t *pstcBuf);
uint32_t BUF_UsedSize(const stc_ring_buf_t *pstcBuf);

bool BUF_Full(const stc_ring_buf_t *pstcBuf);
bool BUF_Empty(const stc_ring_buf_t *pstcBuf);

uint32_t BUF_Read(stc_ring_buf_t *pstcBuf, uint8_t au8Data[], uint32_t u32Len);
uint32_t BUF_Write(stc_ring_buf_t *pstcBuf, uint8_t au8Data[], uint32_t u32Len);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __RING_BUF_H__ */


/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
