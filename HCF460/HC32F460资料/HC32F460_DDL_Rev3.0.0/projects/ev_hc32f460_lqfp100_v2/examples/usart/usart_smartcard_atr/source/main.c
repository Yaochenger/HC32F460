/**
 *******************************************************************************
 * @file  usart/usart_smartcard_atr/source/main.c
 * @brief This example demonstrates USART receive smart-card ATR.
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
 * @addtogroup USART_Smartcard_ATR
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

/* Smartcard CD pin definition */
#define SMARTCARD_CD_PORT               (GPIO_PORT_E)
#define SMARTCARD_CD_PIN                (GPIO_PIN_07)

/* Smartcard reset pin definition */
#define SMARTCARD_RST_PORT              (GPIO_PORT_A)
#define SMARTCARD_RST_PIN               (GPIO_PIN_00)

/* Smartcard power on pin definition */
#define SMARTCARD_PWREN_PORT            (GPIO_PORT_A)
#define SMARTCARD_PWREN_PIN             (GPIO_PIN_01)

/* Smartcard CD pin operation */
#define IS_CARD_REMOVED()               (PIN_SET == GPIO_ReadInputPins(SMARTCARD_CD_PORT, SMARTCARD_CD_PIN))

/* Smartcard reset pin operation */
#define SMARTCARD_RST_LOW()             (GPIO_ResetPins(SMARTCARD_RST_PORT, SMARTCARD_RST_PIN))
#define SMARTCARD_RST_HIGH()            (GPIO_SetPins(SMARTCARD_RST_PORT, SMARTCARD_RST_PIN))

/* Smartcard power pin operation */
#define SMARTCARD_PWR_ON()              (GPIO_ResetPins(SMARTCARD_PWREN_PORT, SMARTCARD_PWREN_PIN))
#define SMARTCARD_PWR_OFF()             (GPIO_SetPins(SMARTCARD_PWREN_PORT, SMARTCARD_PWREN_PIN))

/* USART RX pin definition */
#define USART_RX_PORT                   (GPIO_PORT_A)
#define USART_RX_PIN                    (GPIO_PIN_03)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_37)

/* USART CK pin definition */
#define USART_CK_PORT                   (GPIO_PORT_D)
#define USART_CK_PIN                    (GPIO_PIN_07)
#define USART_CK_GPIO_FUNC              (GPIO_FUNC_7)

/* USART unit definition */
#define USART_UNIT                      (CM_USART2)
#define USART_FCG_ENABLE()              (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART2, ENABLE))

/* USART interrupt definition */
#define USART_RX_ERR_IRQn               (INT000_IRQn)
#define USART_RX_ERR_INT_SRC            (INT_SRC_USART2_EI)

#define USART_RX_FULL_IRQn              (INT001_IRQn)
#define USART_RX_FULL_INT_SRC           (INT_SRC_USART2_RI)

/* Smart-card initial character TS: 0x3B or 0x3F */
#define SMARTCARD_INITIAL_CHARACTER_TS_DIRECT_CONVENTION    (0x3BU)
#define SMARTCARD_INITIAL_CHARACTER_TS_INVERSE_CONVENTION   (0x3FU)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t m_au8RxBuf[100];
static uint32_t m_u32RxIndex;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  USART RX IRQ callback
 * @param  None
 * @retval None
 */
static void USART_RxFull_IrqCallback(void)
{
    uint8_t u8Data = (uint8_t)USART_ReadData(USART_UNIT);

    if (m_u32RxIndex < sizeof(m_au8RxBuf)) {
        m_au8RxBuf[m_u32RxIndex] = u8Data;
        m_u32RxIndex += 1UL;
    }
}

/**
 * @brief  USART error IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxError_IrqCallback(void)
{
    if (SET == USART_GetStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR))) {
        (void)USART_ReadData(USART_UNIT);
    }

    USART_ClearStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR | USART_FLAG_OVERRUN));
}

/**
 * @brief  Main function of UART smartcard project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_gpio_init_t stcGpioInit;
    stc_irq_signin_config_t stcIrqSigninConfig;
    stc_usart_smartcard_init_t stcSmartCardInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Initialize smart card pin. */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(SMARTCARD_RST_PORT, SMARTCARD_RST_PIN, &stcGpioInit);
    (void)GPIO_Init(SMARTCARD_PWREN_PORT, SMARTCARD_PWREN_PIN, &stcGpioInit);

    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_CK_PORT, USART_CK_PIN, USART_CK_GPIO_FUNC);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    while (IS_CARD_REMOVED()) {
        DDL_DelayMS(200UL);
        BSP_LED_Toggle(LED_RED);
    }

    BSP_LED_Off(LED_RED);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Smartcard : active */
    SMARTCARD_RST_LOW();
    SMARTCARD_PWR_ON();
    USART_DeInit(USART_UNIT);

    /* Initialize smartcard. */
    (void)USART_SmartCard_StructInit(&stcSmartCardInit);
    stcSmartCardInit.u32CKOutput = USART_CK_OUTPUT_ENABLE;
    stcSmartCardInit.u32StopBit = USART_STOPBIT_2BIT;
    if (LL_OK == USART_SmartCard_Init(USART_UNIT, &stcSmartCardInit, NULL)) {
        /* Register error IRQ handler && configure NVIC. */
        stcIrqSigninConfig.enIRQn = USART_RX_ERR_IRQn;
        stcIrqSigninConfig.enIntSrc = USART_RX_ERR_INT_SRC;
        stcIrqSigninConfig.pfnCallback = &USART_RxError_IrqCallback;
        (void)INTC_IrqSignIn(&stcIrqSigninConfig);
        NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
        NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

        /* Register RX full IRQ handler && configure NVIC. */
        stcIrqSigninConfig.enIRQn = USART_RX_FULL_IRQn;
        stcIrqSigninConfig.enIntSrc = USART_RX_FULL_INT_SRC;
        stcIrqSigninConfig.pfnCallback = &USART_RxFull_IrqCallback;
        (void)INTC_IrqSignIn(&stcIrqSigninConfig);
        NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
        NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

        /* Enable RX function */
        USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX), ENABLE);

        /* Smartcard : cold reset*/
        SMARTCARD_RST_HIGH();
        DDL_DelayMS(200UL);  /* Delay for receiving smartcard ATR */

        /* Smartcard : release */
        SMARTCARD_RST_LOW();
        USART_DeInit(USART_UNIT);
        SMARTCARD_PWR_OFF();
    }

    if ((SMARTCARD_INITIAL_CHARACTER_TS_DIRECT_CONVENTION == m_au8RxBuf[0]) || \
            (SMARTCARD_INITIAL_CHARACTER_TS_INVERSE_CONVENTION == m_au8RxBuf[0])) {
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
