/**
 *******************************************************************************
 * @file  intc/intc_nmi_xtalstop/source/main.c
 * @brief Main program NMI_XTALSTOP for the Device Driver Library.
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
 * @addtogroup NMI_XTALSTOP
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define DLY_MS              (500UL)
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
 * @brief  NMI xtal stop IRQ handler
 * @param  None
 * @retval None
 */
static void NMI_XtalStop_IrqHandler(void)
{
    if (SET == NMI_GetNmiStatus(NMI_SRC_XTAL)) {
        NMI_ClearNmiStatus(NMI_SRC_XTAL);
        CLK_ClearXtalStdStatus();
        BSP_LED_Toggle(LED_RED);
        BSP_LED_Off(LED_BLUE);
        DDL_DelayMS(DLY_MS);
    }
}

/**
 * @brief  NMI (XTAL_STOP) initialize.
 * @param  None
 * @retval None
 */
static void NMI_Xtal_Init(void)
{
    stc_nmi_init_t stcNmiInit;
    stc_clock_xtalstd_init_t stcXtalStdInit;
    stc_clock_xtal_init_t stcXtalInit;

    (void)CLK_XtalStructInit(&stcXtalInit);
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);

    (void)CLK_XtalStdStructInit(&stcXtalStdInit);
    stcXtalStdInit.u8State = CLK_XTALSTD_ON;
    stcXtalStdInit.u8Int = CLK_XTALSTD_INT_ON;
    (void)CLK_XtalStdInit(&stcXtalStdInit);

    /* NMI interrupt configure */
    (void)NMI_StructInit(&stcNmiInit);
    stcNmiInit.u32Src = NMI_SRC_XTAL;
    (void)NMI_Init(&stcNmiInit);
}

/**
 * @brief  NMI IRQ Handler.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
    NMI_XtalStop_IrqHandler();
}

/**
 * @brief  Main function of NMI_XTALSTOP project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
    /* BSP LED initialize */
    BSP_LED_Init();
    /* NMI initialize */
    NMI_Xtal_Init();
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_SRAM);
    for (;;) {
        BSP_LED_Off(LED_RED);
        BSP_LED_Toggle(LED_BLUE);
        DDL_DelayMS(DLY_MS);
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
