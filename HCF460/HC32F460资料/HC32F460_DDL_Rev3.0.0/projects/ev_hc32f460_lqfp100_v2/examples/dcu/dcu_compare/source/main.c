/**
 *******************************************************************************
 * @file  dcu/dcu_compare/source/main.c
 * @brief This example demonstrates DCU compare function.
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
 * @addtogroup HC32F460_DDL_Examples
 * @{
 */

/**
 * @addtogroup DCU_Compare
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* DCU unit definition */
#define DCU_UNIT                        (CM_DCU1)
#define DCU_FCG_ENABLE()                (FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DCU1, ENABLE))

/* DCU interrupt definition */
#define DCU_IRQn                        (INT000_IRQn)
#define DCU_INT_SRC                     (INT_SRC_DCU1)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static __IO uint32_t m_u32CompareFlag = 0UL;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  DCU irq callback function.
 * @param  None
 * @retval None
 */
static void DCU_IrqCallback(void)
{
    if (SET == DCU_GetStatus(DCU_UNIT, DCU_FLAG_DATA0_LT_DATA2)) {
        m_u32CompareFlag |= DCU_FLAG_DATA0_LT_DATA2;
    }

    if (SET == DCU_GetStatus(DCU_UNIT, DCU_FLAG_DATA0_EQ_DATA2)) {
        m_u32CompareFlag |= DCU_FLAG_DATA0_EQ_DATA2;
    }

    if (SET == DCU_GetStatus(DCU_UNIT, DCU_FLAG_DATA0_GT_DATA2)) {
        m_u32CompareFlag |= DCU_FLAG_DATA0_GT_DATA2;
    }

    if (SET == DCU_GetStatus(DCU_UNIT, DCU_FLAG_DATA0_LT_DATA1)) {
        m_u32CompareFlag |= DCU_FLAG_DATA0_LT_DATA1;
    }

    if (SET == DCU_GetStatus(DCU_UNIT, DCU_FLAG_DATA0_EQ_DATA1)) {
        m_u32CompareFlag |= DCU_FLAG_DATA0_EQ_DATA1;
    }

    if (SET == DCU_GetStatus(DCU_UNIT, DCU_FLAG_DATA0_GT_DATA1)) {
        m_u32CompareFlag |= DCU_FLAG_DATA0_GT_DATA1;
    }

    DCU_ClearStatus(DCU_UNIT, m_u32CompareFlag);
}

/**
 * @brief  Main function of DCU compare project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_dcu_init_t stcDcuInit;
    int32_t i32Ret = LL_OK;
    stc_irq_signin_config_t stcIrqConfig;
    uint8_t au8Data0Val[5] = {0x00U, 0x22U, 0x44U, 0x66U, 0x88};
    uint8_t au8Data1Val[5] = {0x00U, 0x11U, 0x55U, 0x77U, 0x77};
    uint8_t au8Data2Val[5] = {0x00U, 0x11U, 0x55U, 0x55U, 0x00};

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable peripheral clock */
    DCU_FCG_ENABLE();

    /* Initialize DCU */
    stcDcuInit.u32Mode = DCU_MD_CMP;
    stcDcuInit.u32DataWidth = DCU_DATA_WIDTH_8BIT;
    (void)DCU_Init(DCU_UNIT, &stcDcuInit);

    /* Compare trigger conditon */
    DCU_SetCompareCond(DCU_UNIT, DCU_CMP_TRIG_DATA0);

    /* Register IRQ handler && configure NVIC. */
    stcIrqConfig.enIRQn = DCU_IRQn;
    stcIrqConfig.enIntSrc = DCU_INT_SRC;
    stcIrqConfig.pfnCallback = &DCU_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqConfig);
    NVIC_SetPriority(stcIrqConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqConfig.enIRQn);
    NVIC_EnableIRQ(stcIrqConfig.enIRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Enable DCU compare non-window interrupt */
    DCU_IntCmd(DCU_UNIT, DCU_CATEGORY_CMP_NON_WIN, DCU_INT_CMP_NON_WIN_ALL, ENABLE);

    /* Enable DCU interrupt function */
    DCU_GlobalIntCmd(DCU_UNIT, ENABLE);

    /* DATA0 = DATA1  &&  DATA0 = DATA2 */
    DCU_WriteData8(DCU_UNIT, DCU_DATA1_IDX, au8Data1Val[0]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA2_IDX, au8Data2Val[0]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA0_IDX, au8Data0Val[0]);
    if (m_u32CompareFlag != (DCU_FLAG_DATA0_EQ_DATA1 | DCU_FLAG_DATA0_EQ_DATA2)) {
        i32Ret = LL_ERR;
    }

    /* DATA0 > DATA1  &&  DATA0 > DATA2 */
    m_u32CompareFlag = 0UL;
    DCU_WriteData8(DCU_UNIT, DCU_DATA1_IDX, au8Data1Val[1]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA2_IDX, au8Data2Val[1]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA0_IDX, au8Data0Val[1]);
    if (m_u32CompareFlag != (DCU_FLAG_DATA0_GT_DATA1 | DCU_FLAG_DATA0_GT_DATA2)) {
        i32Ret = LL_ERR;
    }

    /* DATA0 < DATA1  &&  DATA0 < DATA2 */
    m_u32CompareFlag = 0UL;
    DCU_WriteData8(DCU_UNIT, DCU_DATA1_IDX, au8Data1Val[2]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA2_IDX, au8Data2Val[2]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA0_IDX, au8Data0Val[2]);
    if (m_u32CompareFlag != (DCU_FLAG_DATA0_LT_DATA1 | DCU_FLAG_DATA0_LT_DATA2)) {
        i32Ret = LL_ERR;
    }

    DCU_IntCmd(DCU_UNIT, DCU_CATEGORY_CMP_NON_WIN, DCU_INT_CMP_NON_WIN_ALL, DISABLE);

    /* Inside window: DATA2 <= DATA0 <= DATA1 */
    m_u32CompareFlag = 0UL;
    DCU_IntCmd(DCU_UNIT, DCU_CATEGORY_CMP_WIN, DCU_INT_CMP_WIN_INSIDE, ENABLE);
    DCU_WriteData8(DCU_UNIT, DCU_DATA1_IDX, au8Data1Val[3]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA2_IDX, au8Data2Val[3]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA0_IDX, au8Data0Val[3]);
    DCU_IntCmd(DCU_UNIT, DCU_CATEGORY_CMP_WIN, DCU_INT_CMP_WIN_INSIDE, DISABLE);
    if (m_u32CompareFlag != (DCU_FLAG_DATA0_LT_DATA1 | DCU_FLAG_DATA0_GT_DATA2)) {
        i32Ret = LL_ERR;
    }

    /* Outside window: DATA0 > DATA2 and DATA0 > DATA1 */
    m_u32CompareFlag = 0UL;
    DCU_IntCmd(DCU_UNIT, DCU_CATEGORY_CMP_WIN, DCU_INT_CMP_WIN_OUTSIDE, ENABLE);
    DCU_WriteData8(DCU_UNIT, DCU_DATA1_IDX, au8Data1Val[4]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA2_IDX, au8Data2Val[4]);
    DCU_WriteData8(DCU_UNIT, DCU_DATA0_IDX, au8Data0Val[4]);
    if (m_u32CompareFlag != (DCU_FLAG_DATA0_GT_DATA1 | DCU_FLAG_DATA0_GT_DATA2)) {
        i32Ret = LL_ERR;
    }

    if (LL_OK == i32Ret) {
        BSP_LED_On(LED_BLUE);   /* Test pass && meet the expected */
    } else {
        BSP_LED_On(LED_RED);    /* Test fail && don't meet the expected */
    }

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
