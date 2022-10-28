/**
 *******************************************************************************
 * @file  i2c/i2c_master_polling/source/main.c
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
 * @addtogroup I2C_master_polling
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
#define DEVICE_ADDR                     (0x38U)
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
#define AHT10_IIC_ADDR	0x38			//AHT10 IIC地址

#define AHT10_CALIBRATION_CMD 	0xE1 	//校准命令(上电后只需要发送一次)
#define AHT10_NORMAL_CMD 		0xA8 	//正常工作模式
#define AHT10_GET_DATA 			0xAC 	//读取数据命令

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
uint8_t AHT10_Init(void);
float AHT10_Read_Humidity(void);
float AHT10_Read_Temperature(void);
uint8_t AHT10_Read_Data(uint8_t *data, uint8_t len);
uint8_t AHT10_ReadOneByte(void);
uint8_t AHT10_Write_Data(uint8_t cmd, uint8_t *data, uint8_t len);
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8TxBuf[TEST_DATA_LEN];
static uint8_t u8RxBuf[TEST_DATA_LEN];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Master transmit data
 *
 * @param  [in] u16DevAddr          The slave address
 * @param  [in] au8Data             The data array
 * @param  [in] u32Size             Data size
 * @param  [in] u32Timeout          Time out count
 * @retval int32_t:
 *            - LL_OK:              Success
 *            - LL_ERR_TIMEOUT:     Time out
 */
static int32_t I2C_Master_Transmit(uint16_t u16DevAddr, uint8_t const au8Data[], uint32_t u32Size, uint32_t u32Timeout)
{
    int32_t i32Ret;
    I2C_Cmd(I2C_UNIT, ENABLE);

    I2C_SWResetCmd(I2C_UNIT, ENABLE);
    I2C_SWResetCmd(I2C_UNIT, DISABLE);
    i32Ret = I2C_Start(I2C_UNIT, u32Timeout);
    if (LL_OK == i32Ret) {
#ifdef I2C_10BITS_ADDR
        i32Ret = I2C_Trans10BitAddr(I2C_UNIT, u16DevAddr, I2C_DIR_TX, u32Timeout);
#else
        i32Ret = I2C_TransAddr(I2C_UNIT, u16DevAddr, I2C_DIR_TX, u32Timeout);
#endif

        if (LL_OK == i32Ret) {
            i32Ret = I2C_TransData(I2C_UNIT, au8Data, u32Size, u32Timeout);
        }
    }

    (void)I2C_Stop(I2C_UNIT, u32Timeout);
    I2C_Cmd(I2C_UNIT, DISABLE);

    return i32Ret;
}

/**
 * @brief  Master receive data
 *
 * @param  [in] u16DevAddr          The slave address
 * @param  [in] au8Data             The data array
 * @param  [in] u32Size             Data size
 * @param  [in] u32Timeout          Time out count
 * @retval int32_t:
 *            - LL_OK:              Success
 *            - LL_ERR_TIMEOUT:     Time out
 */
static int32_t I2C_Master_Receive(uint16_t u16DevAddr, uint8_t au8Data[], uint32_t u32Size, uint32_t u32Timeout)
{
    int32_t i32Ret;

    I2C_Cmd(I2C_UNIT, ENABLE);
    I2C_SWResetCmd(I2C_UNIT, ENABLE);
    I2C_SWResetCmd(I2C_UNIT, DISABLE);
    i32Ret = I2C_Start(I2C_UNIT, u32Timeout);
    if (LL_OK == i32Ret) {
        if (1UL == u32Size) {
            I2C_AckConfig(I2C_UNIT, I2C_NACK);
        }

#ifdef I2C_10BITS_ADDR
        i32Ret = I2C_Trans10BitAddr(I2C_UNIT, u16DevAddr, I2C_DIR_RX, u32Timeout);
#else
        i32Ret = I2C_TransAddr(I2C_UNIT, u16DevAddr, I2C_DIR_RX, u32Timeout);
#endif

        if (LL_OK == i32Ret) {
            i32Ret = I2C_MasterReceiveDataAndStop(I2C_UNIT, au8Data, u32Size, u32Timeout);
        }

        I2C_AckConfig(I2C_UNIT, I2C_ACK);
    }

    if (LL_OK != i32Ret) {
        (void)I2C_Stop(I2C_UNIT, u32Timeout);
    }
    I2C_Cmd(I2C_UNIT, DISABLE);
    return i32Ret;
}

/**
 * @brief  Initialize the I2C peripheral for master
 * @param  None
 * @retval int32_t:
 *            - LL_OK:                  Success
 *            - LL_ERR_INVD_PARAM:      Invalid parameter
 */
static int32_t Master_Initialize(void)
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

    I2C_BusWaitCmd(I2C_UNIT, ENABLE);

    return i32Ret;
}

/**
 * @brief  Main function
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t  main(void)
{
    uint32_t i;
	 float temperature, humidity;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* BSP initialization */
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    AHT10_Init();
    for (i = 0UL; i < TEST_DATA_LEN; i++) {
        u8TxBuf[i] = (uint8_t)(i + 1U);
    }
    (void)memset(u8RxBuf, (int32_t)0x00U, TEST_DATA_LEN);

    /* Initialize I2C port*/
    GPIO_SetFunc(I2C_SCL_PORT, I2C_SCL_PIN, I2C_GPIO_SCL_FUNC);
    GPIO_SetFunc(I2C_SDA_PORT, I2C_SDA_PIN, I2C_GPIO_SDA_FUNC);

    /* Enable I2C Peripheral*/
    FCG_Fcg1PeriphClockCmd(I2C_FCG_USE, ENABLE);
    /* Initialize I2C peripheral and enable function*/
    if (LL_OK != Master_Initialize()) {
        /* Peripheral registers write protected */
        LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
        /* Initialize error*/
        BSP_LED_Toggle(LED_RED);
        for (;;) {
            ;
        }
    }
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    while(1)
	{
		 temperature = AHT10_Read_Temperature();
         humidity = AHT10_Read_Humidity();
		 DDL_Printf();
	}



}

/**
 * @}
 */
uint8_t AHT10_Write_Data(uint8_t cmd, uint8_t *data, uint8_t len)
{
	uint8_t CMD = cmd;
	I2C_Master_Transmit(AHT10_IIC_ADDR, &CMD, 1,TIMEOUT);
    I2C_Master_Transmit(AHT10_IIC_ADDR, data, len,TIMEOUT);
 
}

uint8_t AHT10_ReadOneByte(void)
{
    uint8_t res = 0;
    I2C_Master_Receive(AHT10_IIC_ADDR, &res, 1, TIMEOUT);
    return res;
}

uint8_t AHT10_Read_Data(uint8_t *data, uint8_t len)
{
	I2C_Master_Receive(AHT10_IIC_ADDR, (uint8_t *)&data, len,TIMEOUT);

    return 0;
}

float AHT10_Read_Temperature(void)
{
    uint8_t res = 0;
    uint8_t cmd[2] = {0, 0};
    uint8_t temp[6];
    float cur_temp;

    res = AHT10_Write_Data(AHT10_GET_DATA, cmd, 2); //发送读取数据命令

    if(res)	return 1;

    res = AHT10_Read_Data(temp, 6);				//读取数据

    if(res)	return 1;

    cur_temp = ((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50;

    return cur_temp;
}

float AHT10_Read_Humidity(void)
{
    uint8_t res = 0;
    uint8_t cmd[2] = {0, 0};
    uint8_t humi[6];
    float cur_humi;

    res = AHT10_Write_Data(AHT10_GET_DATA, cmd, 2); //发送读取数据命令

    if(res)	return 1;

    res = AHT10_Read_Data(humi, 6);				//读取数据

    if(res)	return 1;

    cur_humi = ((humi[1]) << 12 | humi[2] << 4 | (humi[3] & 0xF0)) * 100.0 / (1 << 20);

    return cur_humi;
}

uint8_t AHT10_Init(void)
{
    uint8_t res;
    uint8_t temp[2] = {0, 0};

    Master_Initialize();		

    res = AHT10_Write_Data(AHT10_NORMAL_CMD, temp, 2);

    if(res != 0)	return 1;

    DDL_DelayMS(300);

    temp[0] = 0x08;
    temp[1] = 0x00;
    res = AHT10_Write_Data(AHT10_CALIBRATION_CMD, temp, 2);

    if(res != 0)	return 1;

    DDL_DelayMS(300);

    return 0;
}
/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
