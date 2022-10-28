/**
 *******************************************************************************
 * @file  pwc/pwc_lvd/source/main.c
 * @brief Main program of PWC for the Device Driver Library.
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
 * @addtogroup PWC_Lvd
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define LVD1_INT_SRC    (INT_SRC_LVD1)
#define LVD1_IRQn       (INT003_IRQn)

#define LVD2_INT_SRC    (INT_SRC_LVD2)
#define LVD2_IRQn       (INT002_IRQn)

#define DLY_MS          (500UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void PWC_LVD1_IrqCallback(void);
static void PWC_LVD2_IrqCallback(void);
static void LVD_IntInit(void);
static void LVD_Init(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  LVD1 IRQ callback function
 * @param  None
 * @retval None
 */
static void PWC_LVD1_IrqCallback(void)
{
    uint8_t u8Count = 10U;
    do {
        BSP_LED_Toggle(LED_RED);
        DDL_DelayMS(DLY_MS);
    } while ((--u8Count) != 0U);

    PWC_LVD_ClearStatus(PWC_LVD1_FLAG_DETECT);
}

/**
 * @brief  LVD2 IRQ callback function
 * @param  None
 * @retval None
 */
static void PWC_LVD2_IrqCallback(void)
{
    uint8_t u8Count = 10U;
    do {
        BSP_LED_Toggle(LED_BLUE);
        DDL_DelayMS(DLY_MS);
    } while ((--u8Count) != 0U);

    PWC_LVD_ClearStatus(PWC_LVD2_FLAG_DETECT);
}

/**
 * @brief  LVD interrupt initial
 * @param  None
 * @retval None
 */
static void LVD_IntInit(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Clear LVD flag */
    PWC_LVD_ClearStatus(PWC_LVD1_FLAG_DETECT);
    PWC_LVD_ClearStatus(PWC_LVD2_FLAG_DETECT);

    /* LVD1 IRQ sign-in */
    stcIrqSignConfig.enIntSrc = LVD1_INT_SRC;
    stcIrqSignConfig.enIRQn   = LVD1_IRQn;
    stcIrqSignConfig.pfnCallback = &PWC_LVD1_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    /* LVD2 IRQ sign-in */
    stcIrqSignConfig.enIntSrc = LVD2_INT_SRC;
    stcIrqSignConfig.enIRQn   = LVD2_IRQn;
    stcIrqSignConfig.pfnCallback = &PWC_LVD2_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    /* Enable interrupt. */
    NVIC_ClearPendingIRQ(LVD1_IRQn);
    NVIC_SetPriority(LVD1_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(LVD1_IRQn);

    /* Enable interrupt. */
    NVIC_ClearPendingIRQ(LVD2_IRQn);
    NVIC_SetPriority(LVD2_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(LVD2_IRQn);
}

/**
 * @brief  LVD initial
 * @param  None
 * @retval None
 */
static void LVD_Init(void)
{
    stc_pwc_lvd_init_t  stcPwcLvdInit;

    (void)PWC_LVD_StructInit(&stcPwcLvdInit);
    /* Config LVD */
    /* LVD1: 2.8V */
    stcPwcLvdInit.u32State              = PWC_LVD_ON;
    stcPwcLvdInit.u32CompareOutputState = PWC_LVD_CMP_ON;
    stcPwcLvdInit.u32ExceptionType      = PWC_LVD_EXP_TYPE_INT;
    stcPwcLvdInit.u32ThresholdVoltage   = PWC_LVD_THRESHOLD_LVL6;
    (void)PWC_LVD_Init(PWC_LVD_CH1, &stcPwcLvdInit);
    /* LVD2: 2.3V */
    stcPwcLvdInit.u32ThresholdVoltage   = PWC_LVD_THRESHOLD_LVL1;
    (void)PWC_LVD_Init(PWC_LVD_CH2, &stcPwcLvdInit);
}

/**
 * @brief  Main function of LVD project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_LVD | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System Clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();
    /* LVD init */
    LVD_Init();
    /* LVD interrut init */
    LVD_IntInit();
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

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
