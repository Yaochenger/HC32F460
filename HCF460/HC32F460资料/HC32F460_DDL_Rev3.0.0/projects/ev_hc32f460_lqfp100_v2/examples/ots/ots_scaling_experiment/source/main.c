/**
 *******************************************************************************
 * @file  ots/ots_scaling_experiment/source/main.c
 * @brief Main program OTS scaling experiment for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT            First version
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
 * @addtogroup OTS_Scaling_Experiment
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/* This example will show how to do scaling experiment */

/* Average count. */
#define OTS_AVG_CNT                     (10U)

/* Timeout value. */
#define OTS_TIMEOUT_VAL                 (10000UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void OtsHrcConfig(void);
static void OtsXtalConfig(void);
static void OtsScalingExperiment(const char *strClkSrc);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of OTS scaling experiment project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* The system clock is MRC(8MHz) by default. */

    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Initializes UART for debug printing. Baudrate is 19200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 19200U, BSP_PRINTF_Preinit);
    /* BSP key1 is used as the trigger. */
    BSP_KEY_Init();

    /***************** Configuration end, application start **************/

    for (;;) {
        DDL_Printf("---> Press K1 to start.\r\n");
        while (BSP_KEY_GetStatus(BSP_KEY_1) == RESET) { ; }
        /* 1. HRC 16MHz. */
        OTS_DeInit();
        OtsHrcConfig();
        OtsScalingExperiment("HRC 16MHz");

        /* 2. Change clock to XTAL 8MHz. */
        OTS_DeInit();
        OtsXtalConfig();
        OtsScalingExperiment("XTAL 8MHz");
    }
}

/**
 * @brief  Set HRC as the clock source for OTS.
 * @param  None
 * @retval None
 */
static void OtsHrcConfig(void)
{
    stc_ots_init_t stcOTSInit;

    (void)OTS_StructInit(&stcOTSInit);
    stcOTSInit.u16ClockSrc = OTS_CLK_HRC;
    /* Initials OTS. */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_OTS, ENABLE);
    (void)OTS_Init(&stcOTSInit);

    /* Clock configuration. */
    (void)CLK_HrcCmd(ENABLE);
    (void)CLK_Xtal32Cmd(ENABLE);
    (void)CLK_LrcCmd(ENABLE);
}

/**
 * @brief  Set XTAL as the clock source for OTS.
 * @param  None
 * @retval None
 */
static void OtsXtalConfig(void)
{
    stc_ots_init_t stcOTSInit;

    (void)OTS_StructInit(&stcOTSInit);
    stcOTSInit.u16ClockSrc = OTS_CLK_XTAL;
    /* Initials OTS. */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_OTS, ENABLE);
    (void)OTS_Init(&stcOTSInit);

    /* Clock configuration. */
    (void)CLK_XtalCmd(ENABLE);
    (void)CLK_LrcCmd(ENABLE);
}

/**
 * @brief  OTS scaling experiment entity.
 * @param  [in]  strClkSrc              String of OTS clock source information.
 * @retval None
 */
static void OtsScalingExperiment(const char *strClkSrc)
{
    uint32_t i;
    uint16_t u16Dr1;
    uint16_t u16Dr2;
    uint16_t u16Ecr;

    float32_t f32A;
    float32_t f32SumA = 0.0F;
    DDL_Printf("---> Clock source is %s.\r\n", strClkSrc);
    for (i = 0U; i < OTS_AVG_CNT; i++) {
        DDL_DelayMS(100U);
        if (OTS_ScalingExperiment(&u16Dr1, &u16Dr2, &u16Ecr, &f32A, OTS_TIMEOUT_VAL) == LL_OK) {
            DDL_Printf("DR1 = %u, DR2 = %u, ECR = %u, A = %d/1000000\r\n",
                       u16Dr1, u16Dr2, u16Ecr, (int)(f32A * 1000000.F));
            f32SumA += f32A;
        } else {
            DDL_Printf("OTS fault -- timeout.\r\n");
            for (;;);
        }
    }
    DDL_Printf("%s: A = %d/1000000\r\n", strClkSrc, (int)((f32SumA * 1000000.F) / (float32_t)OTS_AVG_CNT));
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
