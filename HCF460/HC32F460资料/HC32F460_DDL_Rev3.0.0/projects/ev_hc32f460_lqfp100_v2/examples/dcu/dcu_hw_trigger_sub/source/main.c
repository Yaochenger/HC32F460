/**
 *******************************************************************************
 * @file  dcu/dcu_hw_trigger_sub/source/main.c
 * @brief This example demonstrates DCU hardware signal trigger DCU sub function.
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
 * @addtogroup DCU_HW_Trigger_Sub
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

/* DCU hardware trigger signal number */
#define DCU_TRIG_SEL                    (AOS_DCU1)
#define DCU_TRIG_EVT_SRC                (EVT_SRC_AOS_STRG)

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
 * @brief  Main function of DCU hardware trigger sub project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t i;
    stc_dcu_init_t stcDcuInit;
    int32_t i32Ret = LL_OK;
    uint32_t au32Data0Val[4];
    uint32_t au32Data2Val[4];
    uint32_t au32Data1Val[4] = {0x00000000, 0x22222222, 0x22222222, 0x22222222};

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Enable peripheral clock */
    DCU_FCG_ENABLE();

    /* Initialize DCU */
    stcDcuInit.u32Mode = DCU_MD_HW_SUB;
    stcDcuInit.u32DataWidth = DCU_DATA_WIDTH_32BIT;
    (void)DCU_Init(DCU_UNIT, &stcDcuInit);

    /* Set DATA0 initial value. */
    DCU_WriteData32(DCU_UNIT, DCU_DATA0_IDX, 0x88888888UL);

    /* Set hardware trigger source */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    AOS_SetTriggerEventSrc(DCU_TRIG_SEL, DCU_TRIG_EVT_SRC);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    for (i = 0UL; i < ARRAY_SZ(au32Data1Val); i++) {
        DCU_WriteData32(DCU_UNIT, DCU_DATA1_IDX, au32Data1Val[i]);

        /* Start trigger event */
        AOS_SW_Trigger();

        au32Data0Val[i] = DCU_ReadData32(DCU_UNIT, DCU_DATA0_IDX);
        au32Data2Val[i] = DCU_ReadData32(DCU_UNIT, DCU_DATA2_IDX);

        /* Check: DATA0 value == 2 * DATA2 value */
        if (au32Data0Val[i] != (2UL * au32Data2Val[i])) {
            i32Ret = LL_ERR;
            break;
        }
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
