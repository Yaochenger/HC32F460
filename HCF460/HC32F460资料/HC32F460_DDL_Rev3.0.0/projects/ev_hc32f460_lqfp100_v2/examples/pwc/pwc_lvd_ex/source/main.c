/**
 *******************************************************************************
 * @file  pwc/pwc_lvd_ex/source/main.c
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
 * @addtogroup PWC_Lvd_Ex
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define LVD2_IRQn                (INT141_IRQn)

/* LVD Port/Pin definition */
#define LVD2_EXT_PORT            (GPIO_PORT_B)
#define LVD2_EXT_PIN             (GPIO_PIN_02)

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
 * @brief  LVD2 IRQ callback function
 * @param  None
 * @retval None
 */
void PWC_LVD2_IrqHandler(void)
{
    BSP_LED_On(LED_RED);
    PWC_LVD_ClearStatus(PWC_LVD2_FLAG_DETECT);
}

/**
 * @brief  LVD interrupt initial
 * @param  None
 * @retval None
 */
static void LVD_IntInit(void)
{
    /* Clear LVD flag */
    PWC_LVD_ClearStatus(PWC_LVD2_FLAG_DETECT);

    /* Set LVD interrupt. */
    (void)INTC_ShareIrqCmd(INT_SRC_LVD2, ENABLE);

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
    stc_gpio_init_t     stcGpioInit;
    stc_pwc_lvd_init_t  stcPwcLvdInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    GPIO_Init(LVD2_EXT_PORT, LVD2_EXT_PIN, &stcGpioInit);

    (void)PWC_LVD_StructInit(&stcPwcLvdInit);
    /* Config LVD External input */
    stcPwcLvdInit.u32State              = PWC_LVD_ON;
    stcPwcLvdInit.u32CompareOutputState = PWC_LVD_CMP_ON;
    stcPwcLvdInit.u32ExceptionType      = PWC_LVD_EXP_TYPE_INT;
    stcPwcLvdInit.u32ThresholdVoltage   = PWC_LVD_EXTVCC;

    PWC_LVD_ExtInputCmd(ENABLE);
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
