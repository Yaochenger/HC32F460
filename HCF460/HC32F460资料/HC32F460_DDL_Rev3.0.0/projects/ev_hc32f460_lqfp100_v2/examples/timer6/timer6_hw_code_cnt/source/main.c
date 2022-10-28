/**
 *******************************************************************************
 * @file  timer6/timer6_hw_code_cnt/source/main.c
 * @brief This example demonstrates Timer6 count for AB phase position coding
 *        function.
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
 * @addtogroup TIMER6_hw_code_cnt
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

#define TMR6_1_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_08)

#define TMR6_1_PWMB_PORT                (GPIO_PORT_E)
#define TMR6_1_PWMB_PIN                 (GPIO_PIN_08)

#define TEST_IO_A_PORT                  (GPIO_PORT_E)
#define TEST_IO_A_PIN                   (GPIO_PIN_03)

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

static void GenClkIn(void)
{
    uint32_t i;

    uint8_t bAin[17U] = {0U, 1U, 1U, 0U, 0U, 1U, 1U, 0U, 0U, 0U, 0U, 1U, 1U, 0U, 0U, 1U, 1U};
    uint8_t bBin[17U] = {1U, 1U, 0U, 0U, 1U, 1U, 0U, 0U, 1U, 1U, 0U, 0U, 1U, 1U, 0U, 0U, 1U};


    for (i = 0UL; i < 17UL; i++) {

        if (0U == bAin[i]) {
            GPIO_ResetPins(TEST_IO_A_PORT, TEST_IO_A_PIN);
        } else {
            GPIO_SetPins(TEST_IO_A_PORT, TEST_IO_A_PIN);
        }

        if (0U == bBin[i]) {
            GPIO_ResetPins(TEST_IO_B_PORT, TEST_IO_B_PIN);
        } else {
            GPIO_SetPins(TEST_IO_B_PORT, TEST_IO_B_PIN);
        }

        DDL_DelayMS(50UL);
        DDL_Printf("0x%x\r\n", (unsigned int)TMR6_GetCountValue(CM_TMR6_1));
    }
}


/**
 * @brief  Main function of TIMER6 compare output mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_gpio_init_t stcGpioCfg;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    /* Initializes UART for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    DDL_Printf("The count value: \r\n");

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1, ENABLE);

    /* Timer6 PWM port configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_1_PWMB_PORT, TMR6_1_PWMB_PIN, GPIO_FUNC_3);

    /* GPIO configurate */
    (void)GPIO_StructInit(&stcGpioCfg);
    stcGpioCfg.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(TEST_IO_A_PORT, TEST_IO_A_PIN, &stcGpioCfg);
    (void)GPIO_Init(TEST_IO_B_PORT, TEST_IO_B_PIN, &stcGpioCfg);

    /* De-initialize */
    TMR6_DeInit(CM_TMR6_1);

    for (;;) {

        GPIO_ResetPins(TEST_IO_A_PORT, TEST_IO_A_PIN);
        GPIO_SetPins(TEST_IO_B_PORT, TEST_IO_B_PIN);

        DDL_DelayMS(10UL);
        TMR6_HWCountUpCondCmd(CM_TMR6_1, TMR6_CNT_UP_COND_ALL, DISABLE);
        TMR6_HWCountDownCondCmd(CM_TMR6_1, TMR6_CNT_DOWN_COND_ALL, DISABLE);

        TMR6_HWCountUpCondCmd(CM_TMR6_1, TMR6_CNT_UP_COND_PWMB_HIGH_PWMA_RISING, ENABLE); /* PWMA Rising trigger when PWMB is high level */
        TMR6_HWCountDownCondCmd(CM_TMR6_1, TMR6_CNT_DOWN_COND_PWMB_LOW_PWMA_RISING, ENABLE); /* PWMA Rising trigger when PWMB is low level */

        TMR6_Start(CM_TMR6_1);

        GenClkIn();

        TMR6_Stop(CM_TMR6_1);
        TMR6_SetCountValue(CM_TMR6_1, 0UL);

        GPIO_ResetPins(TEST_IO_A_PORT, TEST_IO_A_PIN);
        GPIO_SetPins(TEST_IO_B_PORT, TEST_IO_B_PIN);

        DDL_DelayMS(1000UL);

        TMR6_HWCountUpCondCmd(CM_TMR6_1, TMR6_CNT_UP_COND_ALL, DISABLE);
        TMR6_HWCountDownCondCmd(CM_TMR6_1, TMR6_CNT_DOWN_COND_ALL, DISABLE);

        TMR6_HWCountUpCondCmd(CM_TMR6_1, TMR6_CNT_UP_COND_PWMB_HIGH_PWMA_RISING | TMR6_CNT_UP_COND_PWMB_LOW_PWMA_FAILLING, ENABLE);
        TMR6_HWCountDownCondCmd(CM_TMR6_1, TMR6_CNT_DOWN_COND_PWMA_HIGH_PWMB_RISING | TMR6_CNT_DOWN_COND_PWMA_LOW_PWMB_RISING, ENABLE);

        TMR6_Start(CM_TMR6_1);
        DDL_Printf("========\r\n");
        GenClkIn();

        TMR6_Stop(CM_TMR6_1);
        TMR6_SetCountValue(CM_TMR6_1, 0UL);
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
