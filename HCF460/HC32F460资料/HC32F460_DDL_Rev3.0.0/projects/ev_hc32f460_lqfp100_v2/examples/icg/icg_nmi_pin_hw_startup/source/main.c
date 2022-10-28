/**
 *******************************************************************************
 * @file  icg/icg_nmi_pin_hw_startup/source/main.c
 * @brief Main program of ICG NMI for the Device Driver Library.
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
 * @addtogroup ICG_NMI_Pin
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

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
 * @brief  NMI Pin IRQ Handler
 * @param  None
 * @retval None
 */
static void NMI_PIN_IrqHandler(void)
{
    if (SET == NMI_GetNmiStatus(NMI_SRC_PIN)) {
        NMI_ClearNmiStatus(NMI_SRC_PIN);
        BSP_LED_Toggle(LED_RED);
    }
}

/**
 * @brief  NMI IRQ Handler.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
    NMI_PIN_IrqHandler();
}

/**
 * @brief  Main function of ICG NMI Pin function.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_nmi_init_t stcNmiInit;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    BSP_LED_Init();
    /* NMI structure initialize */
    (void)NMI_StructInit(&stcNmiInit);
    /* NMI interrupt configure */
    stcNmiInit.u32Src = NMI_SRC_PIN;
    (void)NMI_Init(&stcNmiInit);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

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
