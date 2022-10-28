/**
 *******************************************************************************
 * @file  timer6/timer6_hw_sta_stp_clr/source/main.c
 * @brief This example demonstrates Timer6 hardware trigger function.
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
 * @addtogroup TIMER6_Hardware_StaStpClr
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU |\
                                         LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

#define TMR6_1_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_08)

#define TMR6_1_PWMB_PORT                (GPIO_PORT_E)
#define TMR6_1_PWMB_PIN                 (GPIO_PIN_08)

#define TEST_IO_A_PORT                  (GPIO_PORT_E)
#define TEST_IO_A_PIN                   (GPIO_PIN_06)

#define TEST_IO_B_PORT                  (GPIO_PORT_D)
#define TEST_IO_B_PIN                   (GPIO_PIN_07)

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
 * @brief  TIMER6 overflow interrupt handler callback.
 * @param  None
 * @retval None
 */
static void Tmr6_OverFlow_CallBack(void)
{
    BSP_LED_Toggle(LED_BLUE);
}

/**
 * @brief  Printf CNTR register and delay.
 * @param  None
 * @retval None
 */
static void TestPrintCountReg(uint8_t u8Num)
{
    uint32_t i;
    for (i = 0U; i < u8Num; i++) {
        DDL_Printf("0x%x\r\n", (unsigned int)TMR6_GetCountValue(CM_TMR6_1));
        DDL_DelayMS(100UL);
    }
}

/**
 * @brief  Main function of TIMER6 compare output mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_timer6_init_t stcTmr6Init;
    stc_gpio_init_t stcGpioCfg;
    stc_irq_signin_config_t stcIrqRegiConf;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    /* Initializes UART for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);

    (void)TMR6_StructInit(&stcTmr6Init);

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1, ENABLE);

    /* Timer6 PWM output port configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_1_PWMB_PORT, TMR6_1_PWMB_PIN, GPIO_FUNC_3);

    /* GPIO configurate */
    (void)GPIO_StructInit(&stcGpioCfg);
    stcGpioCfg.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(TEST_IO_A_PORT, TEST_IO_A_PIN, &stcGpioCfg);
    (void)GPIO_Init(TEST_IO_B_PORT, TEST_IO_B_PIN, &stcGpioCfg);
    GPIO_ResetPins(TEST_IO_A_PORT, TEST_IO_A_PIN);
    GPIO_ResetPins(TEST_IO_B_PORT, TEST_IO_B_PIN);

    /* De-initialize */
    TMR6_DeInit(CM_TMR6_1);
    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV1024;
    stcTmr6Init.u32PeriodValue = HCLK_VALUE / 1024U / 4U;  /* Count for 250ms */
    (void)TMR6_Init(CM_TMR6_1, &stcTmr6Init);

    /* Filter configuration */
    (void)TMR6_SetFilterClockDiv(CM_TMR6_1, TMR6_IO_PWMA, TMR6_FILTER_CLK_DIV16);
    (void)TMR6_SetFilterClockDiv(CM_TMR6_1, TMR6_IO_PWMB, TMR6_FILTER_CLK_DIV16);
    TMR6_FilterCmd(CM_TMR6_1, TMR6_IO_PWMA, ENABLE);
    TMR6_FilterCmd(CM_TMR6_1, TMR6_IO_PWMB, ENABLE);

    /* Configurate hardware start,stop and clear function */
    TMR6_HWStartCondCmd(CM_TMR6_1, TMR6_START_COND_PWMA_RISING, ENABLE);
    TMR6_HWStopCondCmd(CM_TMR6_1, TMR6_STOP_COND_PWMA_FAILLING, ENABLE);
    TMR6_HWClearCondCmd(CM_TMR6_1, TMR6_CLR_COND_PWMB_RISING, ENABLE);

    /* Command hardware start,stop and clear function  */
    TMR6_HWStartCmd(CM_TMR6_1, ENABLE);
    TMR6_HWStopCmd(CM_TMR6_1, ENABLE);
    TMR6_HWClearCmd(CM_TMR6_1, ENABLE);

    /* Configurate interrupt */
    TMR6_IntCmd(CM_TMR6_1, TMR6_INT_OVF, ENABLE);

    stcIrqRegiConf.enIRQn = INT002_IRQn;
    stcIrqRegiConf.enIntSrc = INT_SRC_TMR6_1_OVF;
    stcIrqRegiConf.pfnCallback = &Tmr6_OverFlow_CallBack;
    (void)INTC_IrqSignIn(&stcIrqRegiConf);

    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    for (;;) {
        DDL_DelayMS(100UL);

        GPIO_SetPins(TEST_IO_A_PORT, TEST_IO_A_PIN); /* Hardware start count */
        DDL_Printf("Start count, CNTER =\r\n");
        TestPrintCountReg(20U);

        GPIO_ResetPins(TEST_IO_A_PORT, TEST_IO_A_PIN); /* Hardware stop count */
        DDL_Printf("Stop count, CNTER =\r\n");
        TestPrintCountReg(5U);

        GPIO_SetPins(TEST_IO_B_PORT, TEST_IO_B_PIN); /* Hardware clear count register */
        DDL_Printf("Clear count, CNTER =\r\n");
        TestPrintCountReg(5U);
        GPIO_ResetPins(TEST_IO_B_PORT, TEST_IO_B_PIN); /* Hardware clear count register */
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
