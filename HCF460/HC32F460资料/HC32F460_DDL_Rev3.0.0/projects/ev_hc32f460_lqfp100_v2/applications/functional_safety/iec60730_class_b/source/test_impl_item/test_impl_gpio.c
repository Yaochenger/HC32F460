/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/test_impl_gpio.c
 * @brief This file provides firmware functions to manage the interrupt test.
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
#include "hc32_ll.h"
#include "stl_utility.h"
#include "stl_bsp_conf.h"
#include "test_impl_gpio.h"

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @defgroup Test_Implement_GPIO Test Implement GPIO
 * @{
 */

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

typedef struct stc_test_impl_port_input {
    uint8_t  u8Port;
    uint16_t u16PinAll;
    uint16_t u16PinTest;
    uint16_t u16ExpectVal;
} stc_test_impl_port_input_t;

typedef struct stc_test_impl_port_output {
    uint8_t  u8Port;
    uint16_t u16PinAll;
    uint16_t u16PinTest;
    uint16_t u16OutputVal;
    uint16_t u16ExpectVal;
} stc_test_impl_port_output_t;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
const static stc_test_impl_port_input_t m_astcPortInputTable[] = {
    {STL_INPUT_PORTx, STL_INPUT_PORTx_ALL_PINS, STL_INPUT_PORTx_TEST_PINS, STL_INPUT_PORTx_EXPECT_VAL},
};

const static stc_test_impl_port_output_t m_astcPortOutputTable[] = {
    {STL_OUTPUT_PORTx, STL_OUTPUT_PORTx_ALL_PINS, STL_OUTPUT_PORTx_TEST_PINS, STL_OUTPUT_PORTx_OUT_VAL, STL_OUTPUT_PORTx_EXPECT_VAL},
};

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @defgroup Test_Implement_GPIO_Global_Functions Test Implement GPIO Global Functions
 * @{
 */

/**
 * @brief  GPIO output test initialize in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Initialize successfully.
 *           - STL_ERR:         Initialize unsuccessfully.
 */
uint32_t STL_GpioOutputRuntimeInit(void)
{
    uint8_t i;
    uint8_t j;
    uint16_t u16TestPins;
    uint8_t u8CurrPort;
    uint16_t u16CurrPin;
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;

    for (i = 0U; i < ARRAY_SZ(m_astcPortOutputTable); i++) {
        u8CurrPort = m_astcPortOutputTable[i].u8Port;
        u16TestPins = m_astcPortOutputTable[i].u16PinTest;
        for (j = 0; j < 16U; j++) {
            u16CurrPin = (uint16_t)(1UL << j);
            if ((u16CurrPin & u16TestPins & m_astcPortOutputTable[i].u16PinAll) != 0U) {
                (void)GPIO_Init(u8CurrPort, u16CurrPin, &stcGpioInit);
            }
        }
    }
    return STL_OK;
}

/**
 * @brief  GPIO output test in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Test pass.
 *           - STL_ERR:         Test fail.
 */
uint32_t STL_GpioOutputRuntimeTest(void)
{
    uint8_t i;
    uint8_t u8CurrPort;
    uint16_t u16PinsVal;
    uint32_t u32Ret = STL_OK;

    for (i = 0U; i < ARRAY_SZ(m_astcPortOutputTable); i++) {
        u8CurrPort = m_astcPortOutputTable[i].u8Port;
        u16PinsVal = m_astcPortOutputTable[i].u16OutputVal;
        GPIO_WritePort(u8CurrPort, u16PinsVal);

        u16PinsVal = GPIO_ReadInputPort(u8CurrPort);
        if ((u16PinsVal & m_astcPortOutputTable[i].u16PinTest) != m_astcPortOutputTable[i].u16ExpectVal) {
            u32Ret = STL_ERR;
        }
    }

    return u32Ret;
}

/**
 * @brief  GPIO input test initialize in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Initialize successfully.
 *           - STL_ERR:         Initialize unsuccessfully.
 */
uint32_t STL_GpioInputRuntimeInit(void)
{
    uint8_t i;
    uint8_t j;
    uint16_t u16TestPins;
    uint8_t u8CurrPort;
    uint16_t u16CurrPin;
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);

    for (i = 0U; i < ARRAY_SZ(m_astcPortInputTable); i++) {
        u8CurrPort = m_astcPortInputTable[i].u8Port;
        u16TestPins = m_astcPortInputTable[i].u16PinTest;
        for (j = 0U; j < 16U; j++) {
            u16CurrPin = (uint16_t)(1UL << j);
            if ((u16CurrPin & u16TestPins & m_astcPortInputTable[i].u16PinAll) != 0U) {
                (void)GPIO_Init(u8CurrPort, u16CurrPin, &stcGpioInit);
            }
        }
    }
    return STL_OK;
}

/**
 * @brief  GPIO input test in runtime.
 * @param  None
 * @retval uint32_t:
 *           - STL_OK:          Test pass.
 *           - STL_ERR:         Test fail.
 */
uint32_t STL_GpioInputRuntimeTest(void)
{
    uint8_t i;
    uint16_t PinsVal;
    uint32_t u32Ret = STL_OK;

    for (i = 0U; i < ARRAY_SZ(m_astcPortInputTable); i++) {
        PinsVal = GPIO_ReadInputPort(m_astcPortInputTable[i].u8Port);
        if ((PinsVal & m_astcPortInputTable[i].u16PinTest) != m_astcPortInputTable[i].u16ExpectVal) {
            u32Ret = STL_ERR;
        }
    }

    return u32Ret;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
