/**
 *******************************************************************************
 * @file  clk/clk_switch_sysclk/source/main.c
 * @brief Main program of CLK for the Device Driver Library.
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
 * @addtogroup CLK_Switch_sysclk
 * @{
 */
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define MCO_PORT            GPIO_PORT_A
#define MCO_PIN             GPIO_PIN_08
#define MCO_GPIO_FUNC       GPIO_FUNC_1

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static uint8_t au8SysClockTbl[] = {
    CLK_SYSCLK_SRC_HRC,
    CLK_SYSCLK_SRC_MRC,
    CLK_SYSCLK_SRC_LRC,
    CLK_SYSCLK_SRC_XTAL,
    CLK_SYSCLK_SRC_XTAL32,
    CLK_SYSCLK_SRC_PLL,
};

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void MCOInit(void);
static void XtalInit(void);
static void Xtal32Init(void);
static void MPLLInit(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  MCO pin initialize
 * @param  None
 * @retval None
 */
static void MCOInit(void)
{
    /* Configure clock output pin */
    GPIO_SetFunc(MCO_PORT, MCO_PIN, MCO_GPIO_FUNC);
    /* Configure clock output system clock */
    CLK_MCOConfig(CLK_MCO1, CLK_MCO_SRC_HCLK, CLK_MCO_DIV8);
    /* MCO1 output enable */
    CLK_MCOCmd(CLK_MCO1, ENABLE);
}

/**
 * @brief  Xtal initialize
 * @param  None
 * @retval None
 */
static void XtalInit(void)
{
    stc_clock_xtal_init_t stcXtalInit;

    /* XTAL config */
    (void)CLK_XtalStructInit(&stcXtalInit);
    /* Config Xtal and Enable Xtal */
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8Mode = CLK_XTAL_MD_OSC;
    stcXtalInit.u8Drv = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);
}

/**
 * @brief  Xtal32 initialize
 * @param  None
 * @retval None
 */
static void Xtal32Init(void)
{
    stc_clock_xtal32_init_t stcXtal32Init;

    /* Xtal32 config */
    (void)CLK_Xtal32StructInit(&stcXtal32Init);
    stcXtal32Init.u8State = CLK_XTAL32_ON;
    stcXtal32Init.u8Drv = CLK_XTAL32_DRV_MID;
    stcXtal32Init.u8Filter = CLK_XTAL32_FILTER_ALL_MD;
    (void)CLK_Xtal32Init(&stcXtal32Init);
}

/**
 * @brief  MPLL initialize
 * @param  None
 * @retval None
 */
static void MPLLInit(void)
{
    stc_clock_pll_init_t      stcMPLLInit;

    (void)CLK_PLLStructInit(&stcMPLLInit);
    /* MPLL config */
    /* 8MHz/M*N = 8/1*50/2 = 200MHz */
    stcMPLLInit.PLLCFGR = 0UL;
    stcMPLLInit.PLLCFGR_f.PLLM = (1UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLN = (50UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLP = (2UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLQ = (2UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLR = (2UL - 1UL);
    stcMPLLInit.u8PLLState = CLK_PLL_ON;
    stcMPLLInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;     /* Xtal = 8MHz */
    (void)CLK_PLLInit(&stcMPLLInit);
}

/**
 * @brief  Main function of CLK switch project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t i = 0U;

    /* Register write unprotected for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM);
    /* Set bus clock div. */
    CLK_SetClockDiv(CLK_BUS_CLK_ALL, (CLK_HCLK_DIV1 | CLK_EXCLK_DIV2 | CLK_PCLK0_DIV1 | CLK_PCLK1_DIV2 | \
                                      CLK_PCLK2_DIV4 | CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2));
    /* BSP key initialize */
    BSP_KEY_Init();
    /* sram init include read/write wait cycle setting */
    SRAM_SetWaitCycle(SRAM_SRAM_ALL, SRAM_WAIT_CYCLE1, SRAM_WAIT_CYCLE1);
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    /* flash read wait cycle setting */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    /* output system clock */
    MCOInit();
    /* Xtal initialize */
    XtalInit();
    /* enable Xtal32 */
    Xtal32Init();
    /* MPLL initialize */
    MPLLInit();
    /* enable LRC */
    (void)CLK_LrcCmd(ENABLE);
    /* enable HRC */
    (void)CLK_HrcCmd(ENABLE);
    /* Switch driver ability */
    PWC_HighSpeedToHighPerformance();
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_EFM | LL_PERIPH_GPIO | LL_PERIPH_SRAM);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_10)) {
            CLK_SetSysClockSrc(au8SysClockTbl[i]);
            while (RESET != BSP_KEY_GetStatus(BSP_KEY_10));
            i++;
            if (i >= sizeof(au8SysClockTbl) / sizeof(au8SysClockTbl[0U])) {
                i = 0U;
            }
        }
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
