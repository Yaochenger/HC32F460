/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file main.c
 **
 ** \brief This example demonstrates how to use DCU sub operation function.
 **
 **   - 2018-10-15 CDT First version for Device Driver Library of DCU
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define DCU_UNIT                        (M4_DCU1)

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
 *******************************************************************************
 ** \brief  Main function of project
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 **
 ******************************************************************************/
int32_t main(void)
{
    uint32_t i;
    stc_dcu_init_t stcDcuInit;
    en_result_t enTestResult = Ok;
    uint32_t au32Data0Val[4];
    uint32_t au32Data2Val[4];
    uint32_t au32Data1Val[4] = {0x00000000, 0x22222222, 0x22222222, 0x22222222};
    uint32_t u32CalTimes = sizeof(au32Data1Val)/4ul;

    /* Initialize LED */
    BSP_LED_Init();

    /* Enable peripheral clock */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DCU1, Enable);

    /* Initialize DCU */
    MEM_ZERO_STRUCT(stcDcuInit);
    stcDcuInit.u32IntSel = 0u;
    stcDcuInit.enIntWinMode = DcuIntInvalid;
    stcDcuInit.enDataSize = DcuDataBit32;
    stcDcuInit.enOperation = DcuOpSub;
    DCU_Init(DCU_UNIT, &stcDcuInit);
    DCU_WriteDataWord(DCU_UNIT, DcuRegisterData0, 0x88888888ul);

    for (i = 0u; i < u32CalTimes; i++)
    {
        DCU_WriteDataWord(DCU_UNIT, DcuRegisterData1, au32Data1Val[i]);

        au32Data0Val[i] = DCU_ReadDataWord(DCU_UNIT, DcuRegisterData0);
        au32Data2Val[i] = DCU_ReadDataWord(DCU_UNIT, DcuRegisterData2);

        /* Compare DCU regisger DATA0 && DATA2 value: DATA0 value == 2 * DATA2 value */
        if (au32Data0Val[i] != (2ul * au32Data2Val[i]))
        {
            enTestResult = Error;
            break;
        }
        else
        {
        }

    }

    if (Ok == enTestResult)
    {
        BSP_LED_On(LED_GREEN);  /* Test pass && meet the expected */
    }
    else
    {
        BSP_LED_On(LED_RED);  /* Test fail && don't meet the expected */
    }

    while (1)
    {
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
