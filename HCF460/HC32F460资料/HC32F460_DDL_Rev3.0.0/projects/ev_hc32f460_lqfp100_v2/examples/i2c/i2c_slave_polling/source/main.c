/**
 *******************************************************************************
 * @file  i2c/i2c_slave_polling/source/main.c
 * @brief Main program of I2C for the Device Driver Library.
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
 * @addtogroup I2C_slave_polling
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

/* Define I2C unit used for the example */
#define I2C_UNIT                        (CM_I2C3)
#define I2C_FCG_USE                     (FCG1_PERIPH_I2C3)
/* Define slave device address for example */
#define DEVICE_ADDR                     (0x06U)
/* If I2C 10 bit address, open the define for I2C_10BITS_ADDR */
//#define I2C_10BITS_ADDR               (1U)

/* Define port and pin for SDA and SCL */
#define I2C_SCL_PORT                    (GPIO_PORT_E)
#define I2C_SCL_PIN                     (GPIO_PIN_15)
#define I2C_SDA_PORT                    (GPIO_PORT_B)
#define I2C_SDA_PIN                     (GPIO_PIN_05)
#define I2C_GPIO_SCL_FUNC               (GPIO_FUNC_49)
#define I2C_GPIO_SDA_FUNC               (GPIO_FUNC_48)

#define TIMEOUT                         (0x40000UL)

/* Define Write and read data length for the example */
#define TEST_DATA_LEN                   (256U)
/* Define i2c baudrate */
#define I2C_BAUDRATE                    (400000UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8RxBuf[TEST_DATA_LEN];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Slave receive data
 *
 * @param  au8Data               Data array
 * @param  u32Size               Data size
 * @param  u32Timeout            Time out count
 * @retval int32_t:
 *            - LL_OK:                 Success
 *            - LL_ERR:              Failed
 *            - LL_ERR_TIMEOUT:       Time out
 */
static int32_t I2C_Slave_Receive(uint8_t au8Data[], uint32_t u32Size, uint32_t u32Timeout)
{
    int32_t i32Ret;
    /* clear all status */

    I2C_Cmd(I2C_UNIT, ENABLE);

    /* Clear status */
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_STOPFCLR | I2C_CLR_NACKFCLR);

    /* Wait slave address matched */
    while (RESET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_MATCH_ADDR0)) {
        ;
    }
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_SLADDR0FCLR);

    if (RESET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_TRA)) {
        /* Slave receive data*/
        i32Ret = I2C_ReceiveData(I2C_UNIT, au8Data, u32Size, u32Timeout);

        if ((LL_OK == i32Ret) || (LL_ERR_TIMEOUT == i32Ret)) {
            /* Wait stop condition */
            i32Ret = I2C_WaitStatus(I2C_UNIT, I2C_FLAG_STOP, SET, u32Timeout);
        }
    } else {
        i32Ret = LL_ERR;
    }

    I2C_Cmd(I2C_UNIT, DISABLE);
    return i32Ret;
}

/**
 * @brief  Slave transmit data
 **
 * @param  au8Data               Data array
 * @param  u32Size               Data size
 * @param  u32Timeout            Time out count
 * @retval int32_t:
 *            - LL_OK:                 Success
 *            - LL_ERR:              Failed
 *            - LL_ERR_TIMEOUT:       Time out
 */
static int32_t I2C_Slave_Transmit(uint8_t au8Data[], uint32_t u32Size, uint32_t u32Timeout)
{
    int32_t i32Ret;

    I2C_Cmd(I2C_UNIT, ENABLE);

    /* Clear status */
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_STOPFCLR | I2C_CLR_NACKFCLR);

    /* Wait slave address matched */
    while (RESET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_MATCH_ADDR0)) {
        ;
    }
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_SLADDR0FCLR);

#ifdef I2C_10BITS_ADDR
    /* Wait slave address matched */
    while (RESET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_MATCH_ADDR0)) {
        ;
    }
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_SLADDR0FCLR);
#endif

    if (RESET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_TRA)) {
        i32Ret = LL_ERR;
    } else {
        i32Ret = I2C_TransData(I2C_UNIT, au8Data, u32Size, u32Timeout);

        if ((LL_OK == i32Ret) || (LL_ERR_TIMEOUT == i32Ret)) {
            /* Release SCL pin */
            (void)I2C_ReadData(I2C_UNIT);

            /* Wait stop condition */
            i32Ret = I2C_WaitStatus(I2C_UNIT, I2C_FLAG_STOP, SET, u32Timeout);
        }
    }

    I2C_Cmd(I2C_UNIT, DISABLE);
    return i32Ret;
}


/**
 * @brief  Initialize the I2C peripheral for slave
 * @param  None
 * @retval int32_t:
 *            - LL_OK:                 Success
 *            - LL_ERR_INVD_PARAM:  Invalid parameter
 */
static int32_t Slave_Initialize(void)
{
    int32_t i32Ret;
    stc_i2c_init_t stcI2cInit;
    float32_t fErr;

    I2C_DeInit(I2C_UNIT);

    (void)I2C_StructInit(&stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV2;
    stcI2cInit.u32Baudrate = I2C_BAUDRATE;
    stcI2cInit.u32SclTime = 3UL;
    i32Ret = I2C_Init(I2C_UNIT, &stcI2cInit, &fErr);

    if (LL_OK == i32Ret) {
        /* Set slave address*/
#ifdef I2C_10BITS_ADDR
        I2C_SlaveAddrConfig(I2C_UNIT, I2C_ADDR0, I2C_ADDR_10BIT, DEVICE_ADDR);
#else
        I2C_SlaveAddrConfig(I2C_UNIT, I2C_ADDR0, I2C_ADDR_7BIT, DEVICE_ADDR);
#endif
    }
    return i32Ret;
}

/**
 * @brief  Main function
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t  main(void)
{
    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* BSP initialization */
    BSP_CLK_Init();
    BSP_LED_Init();

    (void)memset(u8RxBuf, (int32_t)0x00U, TEST_DATA_LEN);

    /* Initialize I2C port*/
    GPIO_SetFunc(I2C_SCL_PORT, I2C_SCL_PIN, I2C_GPIO_SCL_FUNC);
    GPIO_SetFunc(I2C_SDA_PORT, I2C_SDA_PIN, I2C_GPIO_SDA_FUNC);

    /* Enable I2C Peripheral*/
    FCG_Fcg1PeriphClockCmd(I2C_FCG_USE, ENABLE);
    /* Initialize I2C peripheral and enable function*/
    if (LL_OK != Slave_Initialize()) {
        /* Peripheral registers write protected */
        LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
        /* Initialize error*/
        BSP_LED_On(LED_RED);
        for (;;) {
            ;
        }
    }

    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (LL_OK == I2C_Slave_Receive(u8RxBuf, TEST_DATA_LEN, TIMEOUT)) {
            if (LL_OK != I2C_Slave_Transmit(u8RxBuf, TEST_DATA_LEN, TIMEOUT)) {
                /* Failed */
                break;
            }
        } else {
            /* Failed */
            break;
        }
    }

    /* Communication failed */
    BSP_LED_Off(LED_BLUE);
    BSP_LED_On(LED_RED);
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
