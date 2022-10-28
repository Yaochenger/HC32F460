/**
 *******************************************************************************
 * @file  dcu/dcu_sub/source/main.c
 * @brief This example demonstrates DCU sub function.
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
 * @addtogroup DCU_Sub
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
static __IO uint32_t m_u32AddUnderflowCount = 0UL;

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
    if (SET == DCU_GetStatus(DCU_UNIT, DCU_FLAG_CARRY)) {
        m_u32AddUnderflowCount++;
        DCU_ClearStatus(DCU_UNIT, DCU_FLAG_CARRY);
    }
}

/**
 * @brief  Main function of DCU sub project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t i;
    stc_dcu_init_t stcDcuInit;
    int32_t i32Ret = LL_OK;
    stc_irq_signin_config_t stcIrqConfig;
    uint32_t u32SumData1 = 0UL;
    const uint32_t u32Data0InitValue = 0x88888888UL;
    uint32_t u32UnderflowData0 = 0UL;
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
    stcDcuInit.u32Mode = DCU_MD_SUB;
    stcDcuInit.u32DataWidth = DCU_DATA_WIDTH_32BIT;
    (void)DCU_Init(DCU_UNIT, &stcDcuInit);

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

    /* Enable DCU operation interrupt */
    DCU_IntCmd(DCU_UNIT, DCU_CATEGORY_OP, DCU_INT_OP_CARRY, ENABLE);

    /* Enable DCU interrupt function */
    DCU_GlobalIntCmd(DCU_UNIT, ENABLE);

    DCU_WriteData32(DCU_UNIT, DCU_DATA0_IDX, u32Data0InitValue);

    for (i = 0UL; i < ARRAY_SZ(au32Data1Val); i++) {
        u32SumData1 += au32Data1Val[i];
        DCU_WriteData32(DCU_UNIT, DCU_DATA1_IDX, au32Data1Val[i]);

        au32Data0Val[i] = DCU_ReadData32(DCU_UNIT, DCU_DATA0_IDX);
        au32Data2Val[i] = DCU_ReadData32(DCU_UNIT, DCU_DATA2_IDX);

        /* Check: DATA0 value == 2 * DATA2 value */
        if (au32Data0Val[i] != (2UL * au32Data2Val[i])) {
            i32Ret = LL_ERR;
            break;
        }
    }

    u32SumData1 += 0x22222223UL;
    DCU_WriteData32(DCU_UNIT, DCU_DATA1_IDX, 0x22222223UL);

    /* Wait sub underflow */
    while (0UL == m_u32AddUnderflowCount) {
    }

    u32UnderflowData0 += DCU_ReadData32(DCU_UNIT, DCU_DATA0_IDX);

    /* Check: DATA0 value == 0 - 0x22222222UL */
    if (u32UnderflowData0 != (u32Data0InitValue - u32SumData1)) {
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
