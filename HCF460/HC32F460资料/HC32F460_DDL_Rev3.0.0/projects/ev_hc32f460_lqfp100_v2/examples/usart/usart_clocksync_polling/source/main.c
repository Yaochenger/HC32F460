/**
 *******************************************************************************
 * @file  usart/usart_clocksync_polling/source/main.c
 * @brief This example demonstrates clock sync data receive and transfer by polling.
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
 * @addtogroup USART_ClockSync_Polling
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* USART CK/RX/TX pin definition */
#define USART_CK_PORT                   (GPIO_PORT_D)   /* PD7: USART2_CK */
#define USART_CK_PIN                    (GPIO_PIN_07)
#define USART_CK_GPIO_FUNC              (GPIO_FUNC_7)

#define USART_RX_PORT                   (GPIO_PORT_A)   /* PA3: USART2_RX */
#define USART_RX_PIN                    (GPIO_PIN_03)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_37)

#define USART_TX_PORT                   (GPIO_PORT_A)   /* PA2: USART2_TX */
#define USART_TX_PIN                    (GPIO_PIN_02)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_36)

/* USART unit definition */
#define USART_UNIT                      (CM_USART2)
#define USART_FCG_ENABLE()              (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART2, ENABLE))

/* Communication data size */
#define COM_DATA_LEN                    (ARRAY_SZ(m_au8TxData))

/* DDL_ON: master device / DDL_OFF: slave device */
#define MASTER_DEVICE_ENABLE            (DDL_ON)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static const uint8_t m_au8TxData[] = "USART clock-sync test.";
static uint8_t m_au8RxData[COM_DATA_LEN];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of clock-sync polling project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_usart_clocksync_init_t stcClockSyncInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Initialize BSP key. */
    BSP_KEY_Init();

    /* Configure USART RX/TX pin. */
    GPIO_SetFunc(USART_CK_PORT, USART_CK_PIN, USART_CK_GPIO_FUNC);
    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Initialize CLKSYNC function. */
    (void)USART_ClockSync_StructInit(&stcClockSyncInit);
#if (MASTER_DEVICE_ENABLE == DDL_ON)
    stcClockSyncInit.u32ClockSrc = USART_CLK_SRC_INTERNCLK;
    stcClockSyncInit.u32ClockDiv = USART_CLK_DIV64;
    stcClockSyncInit.u32Baudrate = 9600UL;
#else
    stcClockSyncInit.u32ClockSrc = USART_CLK_SRC_EXTCLK;
#endif
    if (LL_OK != USART_ClockSync_Init(USART_UNIT, &stcClockSyncInit, NULL)) {
        BSP_LED_On(LED_RED);
        for (;;) {
        }
    }

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /* Enable RX/TX function */
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_TX), ENABLE);

    /* Wait key press */
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_1)) {
    }

    /* Start the transmission process*/
    (void)USART_ClockSync_TransReceive(USART_UNIT, m_au8TxData, m_au8RxData, COM_DATA_LEN, USART_MAX_TIMEOUT);

    /* Compare data */
    if (0 == memcmp(m_au8RxData, m_au8TxData, COM_DATA_LEN)) {
        BSP_LED_On(LED_BLUE);
    } else {
        BSP_LED_On(LED_RED);
    }

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
