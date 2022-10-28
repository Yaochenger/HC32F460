/**
 *******************************************************************************
 * @file  efm/efm_int/source/main.c
 * @brief Main program of EFM for the Device Driver Library.
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
 * @addtogroup EFM_Int
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EFM_IRQn                        (INT038_IRQn)
#define EFM_INT_SRC                     (INT_SRC_EFM_PEERR)

#define EFM_WIN_START_ADDR              (0x0007D000u)
#define EFM_WIN_END_ADDR                (0x0007E000u)

#define EFM_SECTOR62_NUM                (62U)

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
 * @brief  EFM Program/Erase Error IRQ callback
 * @param  None
 * @retval None
 */
static void EFM_ProgramEraseError_IrqHandler(void)
{
    BSP_LED_Off(LED_BLUE);
    BSP_LED_On(LED_RED);

    /* Clear Flag */
    EFM_ClearStatus(EFM_FLAG_PEPRTERR);
}

/**
 * @brief  Configure IRQ handler && NVIC.
 * @param  None
 * @retval None
 */
static void EFM_IntInit(void)
{
    stc_irq_signin_config_t stcIrqRegCfg;

    /* Register IRQ handler && configure NVIC. */
    stcIrqRegCfg.enIRQn = EFM_IRQn;
    stcIrqRegCfg.enIntSrc = EFM_INT_SRC;
    stcIrqRegCfg.pfnCallback = &EFM_ProgramEraseError_IrqHandler;
    (void)INTC_IrqSignIn(&stcIrqRegCfg);

    NVIC_ClearPendingIRQ(stcIrqRegCfg.enIRQn);
    NVIC_SetPriority(stcIrqRegCfg.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqRegCfg.enIRQn);

    /* Enable Interrupt function */
    EFM_IntCmd(EFM_INT_PEERR, ENABLE);
}

/**
 * @brief  Main function of EFM project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t u32Addr;
    const uint32_t u32TestData = 0x5A5A5A5AU;

    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* System clock init */
    BSP_CLK_Init();
    /* LED init */
    BSP_LED_Init();
    /* KEY Init */
    BSP_KEY_Init();

    /* Turn on LED blue */
    BSP_LED_On(LED_BLUE);

    /*Configure EFM interrupt */
    EFM_IntInit();

    /* Enable flash. Default value is Enable*/
    EFM_Cmd(EFM_CHIP_ALL, ENABLE);
    /* Wait flash ready. */
    while (SET != EFM_GetStatus(EFM_FLAG_RDY)) {
        ;
    }

    /* EFM_FWMC wirte enable */
    EFM_FWMC_Cmd(ENABLE);
    /* Erase sector 62. */
    (void)EFM_SectorErase(EFM_SECTOR_ADDR(EFM_SECTOR62_NUM));

    /* Set windows protect address. */
    EFM_SetWindowProtectAddr(EFM_WIN_START_ADDR, EFM_WIN_END_ADDR);

    /* program between windows address. */
    u32Addr = EFM_WIN_START_ADDR + 4UL;
    EFM_ProgramWord(u32Addr, u32TestData);

    /* Press KEY1 */
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_1)) {
        ;
    }


    /* program out of windows address. */
    u32Addr = EFM_WIN_START_ADDR - 4UL;
    EFM_ProgramWord(u32Addr, u32TestData);

    EFM_ClearStatus(EFM_FLAG_PEPRTERR);

    EFM_FWMC_Cmd(DISABLE);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    for (;;) {
        ;
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
