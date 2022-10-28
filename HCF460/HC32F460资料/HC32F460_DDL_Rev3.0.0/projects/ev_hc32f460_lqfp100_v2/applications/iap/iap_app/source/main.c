/**
 *******************************************************************************
 * @file  iap/iap_app/source/main.c
 * @brief Main program of IAP Application.
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
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_FCG | LL_PERIPH_SRAM)
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
static uint8_t u8ExtIntFlag = 1U, u8LedState = 0U;

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
    static uint16_t u16TickCnt = 0U;

    SysTick_IncTick();
    if (0U != u8ExtIntFlag) {
        if (++u16TickCnt >= 200U) {
            u16TickCnt = 0U;
            if (0U == u8LedState) {
                BSP_LED_Toggle(LED_RED);
            } else {
                BSP_LED_Toggle(LED_BLUE);
            }
            if (++u8LedState >= 2U) {
                u8LedState = 0U;
            }
        }
    }
}

/**
 * @brief  KEY10 External interrupt handler function
 * @param  None
 * @retval None
 */
void BSP_KEY_KEY10_IrqHandler(void)
{
    if (SET == EXTINT_GetExtIntStatus(BSP_KEY_KEY10_EXTINT)) {
        u8ExtIntFlag = !u8ExtIntFlag;
        if (0U == u8ExtIntFlag) {
            u8LedState = 0U;
            BSP_LED_Off(LED_RED | LED_BLUE);
        }
        EXTINT_ClearExtIntStatus(BSP_KEY_KEY10_EXTINT);
    }
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
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
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
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
}

/**
 * @brief  Main function of SysTick.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    IAP_PeriphInit();

    for (;;) {
        Modem_Process(&ModemCom, &ModemFlash, IAP_COM_WAIT_TIME);
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
