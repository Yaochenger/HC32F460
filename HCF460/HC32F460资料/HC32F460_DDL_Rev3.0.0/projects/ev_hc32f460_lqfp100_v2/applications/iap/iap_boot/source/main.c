/**
 *******************************************************************************
 * @file  iap/iap_boot/source/main.c
 * @brief Main program of IAP Boot.
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

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
/* Communication timeout */
#define IAP_COM_WAIT_TIME               (2000UL)  //ms

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
stc_modem_com_t ModemCom;
stc_modem_flash_t ModemFlash;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint32_t JumpAddr;
static func_ptr_t JumpToApp;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  SysTick interrupt handler function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    SysTick_IncTick();
}

/**
 * @brief  Systick De-Initialize.
 * @param  None
 * @retval None
 */
void SysTick_DeInit(void)
{
    SysTick->CTRL  = 0UL;
    SysTick->LOAD  = 0UL;
    SysTick->VAL   = 0UL;
}

/**
 * @brief  IAP clock initialize.
 *         Set board system clock to PLL@200MHz
 * @param  None
 * @retval None
 */
void IAP_CLK_Init(void)
{
    stc_clock_xtal_init_t stcXtalInit;
    stc_clock_pll_init_t  stcMpllInit;

    /* Set bus clk div. */
    CLK_SetClockDiv(CLK_BUS_CLK_ALL, (CLK_HCLK_DIV1 | CLK_EXCLK_DIV2 | CLK_PCLK0_DIV1 |
                                      CLK_PCLK1_DIV2 | CLK_PCLK2_DIV4 | CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2));

    (void)CLK_XtalStructInit(&stcXtalInit);
    /* Config Xtal and enable Xtal */
    stcXtalInit.u8Mode  = CLK_XTAL_MD_OSC;
    stcXtalInit.u8Drv   = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);

    (void)CLK_PLLStructInit(&stcMpllInit);
    /* MPLL config (XTAL / pllmDiv * plln / PllpDiv = 200M). */
    stcMpllInit.PLLCFGR = 0UL;
    stcMpllInit.PLLCFGR_f.PLLM = 1UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLN = 50UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLP = 2UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLQ = 2UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLR = 2UL - 1UL;
    stcMpllInit.u8PLLState = CLK_PLL_ON;
    stcMpllInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;
    (void)CLK_PLLInit(&stcMpllInit);
    /* Wait MPLL ready. */
    while (SET != CLK_GetStableStatus(CLK_STB_FLAG_PLL)) {
    }

    /* sram init include read/write wait cycle setting */
    SRAM_Init();
    SRAM_SetWaitCycle(SRAM_SRAM_ALL, SRAM_WAIT_CYCLE1, SRAM_WAIT_CYCLE1);
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    /* 3 cycles for 126MHz ~ 200MHz */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT3);
    /* flash read wait cycle setting */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    /* Switch driver ability */
    (void)PWC_HighSpeedToHighPerformance();
    /* Switch system clock source to MPLL. */
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_PLL);
}

/**
 * @brief  IAP clock De-Initialize.
 * @param  None
 * @retval None
 */
void IAP_CLK_DeInit(void)
{
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_MRC);
    /* Switch driver ability */
    (void)PWC_HighPerformanceToLowSpeed();
    /* Set bus clk div. */
    CLK_SetClockDiv(CLK_BUS_CLK_ALL, (CLK_HCLK_DIV1 | CLK_EXCLK_DIV1 | CLK_PCLK0_DIV1 |
                                      CLK_PCLK1_DIV1 | CLK_PCLK2_DIV1 | CLK_PCLK3_DIV1 | CLK_PCLK4_DIV1));
    CLK_XtalCmd(DISABLE);
    CLK_PLLCmd(DISABLE);
    /* sram init include read/write wait cycle setting */
    SRAM_SetWaitCycle(SRAM_SRAM_ALL, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    /* 0 cycles */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT0);
    /* flash read wait cycle setting */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE0);
}

/**
 * @brief  CRC Initialize.
 * @param  None
 * @retval None
 */
void IAP_CRC_Init(void)
{
    stc_crc_init_t stcCrcInit;

    /* Enable CRC Clock */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_CRC, ENABLE);
    /* Initialize CRC16 */
    CRC_StructInit(&stcCrcInit);
    CRC_Init(&stcCrcInit);
}

/**
 * @brief  CRC De-Initialize.
 * @param  None
 * @retval None
 */
void IAP_CRC_DeInit(void)
{
    CRC_DeInit();
    /* Disable CRC Clock */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_CRC, DISABLE);
}

/**
 * @brief  IAP peripheral initialize.
 * @param  None
 * @retval None
 */
void IAP_PeriphInit(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    EFM_FWMC_Cmd(ENABLE);
    /* Init Peripheral */
    IAP_CLK_Init();
    SysTick_Init(1000U);
    COM_Init();
    IAP_CRC_Init();
    /* Configure COM interface */
    ModemCom.SendData = COM_SendData;
    ModemCom.RecvData = COM_RecvData;
    /* Configure Flash interface */
    ModemFlash.u32FlashBase   = FLASH_BASE;
    ModemFlash.u32FlashSize   = FLASH_SIZE;
    ModemFlash.u32SectorSize  = FLASH_SECTOR_SIZE;
    ModemFlash.CheckAddrAlign = FLASH_CheckAddrAlign;
    ModemFlash.EraseSector    = FLASH_EraseSector;
    ModemFlash.WriteData      = FLASH_WriteData;
    ModemFlash.ReadData       = FLASH_ReadData;
}

/**
 * @brief  IAP peripheral de-initialize.
 * @param  None
 * @retval None
 */
void IAP_PeriphDeinit(void)
{
    /* De-Init Peripheral */
    COM_DeInit();
    IAP_CRC_DeInit();
    SysTick_DeInit();
    IAP_CLK_DeInit();
    /* Peripheral registers write protected */
    EFM_FWMC_Cmd(DISABLE);
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
}

/**
 * @brief  Jump from boot to app function.
 * @param  [in] u32Addr                 APP address
 * @retval LL_ERR                       APP address error
 */
int32_t IAP_JumpToApp(uint32_t u32Addr)
{
    uint32_t u32StackTop = *((__IO uint32_t *)u32Addr);

    /* Check stack top pointer. */
    if ((u32StackTop > SRAM_BASE) && (u32StackTop <= (SRAM_BASE + SRAM_SIZE))) {
        IAP_PeriphDeinit();
        /* Jump to user application */
        JumpAddr = *(__IO uint32_t *)(u32Addr + 4U);
        JumpToApp = (func_ptr_t)JumpAddr;
        /* Initialize user application's Stack Pointer */
        __set_MSP(u32StackTop);
        JumpToApp();
    }

    return LL_ERR;
}

/**
 * @brief  IAP check app.
 * @param  None
 * @retval None
 */
void IAP_CheckApp(void)
{
    uint32_t u32AppAddr;

    if ((APP_UPGRADE_FLAG != *(__IO uint32_t *)APP_UPGRADE_FLAG_ADDR)) {
        if ((APP_EXIST_FLAG == *(__IO uint32_t *)APP_EXIST_FLAG_ADDR)) {
            u32AppAddr = *(__IO uint32_t *)APP_RUN_ADDR;
            if ((u32AppAddr >= (FLASH_BASE + IAP_BOOT_SIZE)) && (u32AppAddr < (FLASH_BASE + FLASH_SIZE))) {
                if (LL_OK != IAP_JumpToApp(u32AppAddr)) {
                    /* Jump to app failed */
                }
            }
        }
    }
}

/**
 * @brief  Main function of Boot.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    IAP_PeriphInit();
    /* Check app validity */
    IAP_CheckApp();

    for (;;) {
        if (LL_OK == Modem_Process(&ModemCom, &ModemFlash, IAP_COM_WAIT_TIME)) {
            if (LL_OK != IAP_JumpToApp(*(__IO uint32_t *)APP_RUN_ADDR)) {
                /* Jump to app failed */
            }
        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
