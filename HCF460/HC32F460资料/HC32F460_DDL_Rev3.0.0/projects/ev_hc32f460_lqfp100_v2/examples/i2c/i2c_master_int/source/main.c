/**
 *******************************************************************************
 * @file  i2c/i2c_master_int/source/main.c
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
 * @addtogroup I2C_Master_Int
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
/**
 * @brief I2c communication mode enum
 */
typedef enum {
    MD_TX = 0U,
    MD_RX = 1U,
} stc_i2c_com_mode_t;

/**
 * @brief I2c communication status enum
 */
typedef enum {
    I2C_COM_BUSY = 0U,
    I2C_COM_IDLE = 1U,
} stc_i2c_com_status_t;

/**
 * @brief I2c communication structure
 */
typedef struct {
    stc_i2c_com_mode_t    enMode;         /*!< I2C communication mode*/
    uint32_t              u32Len;         /*!< I2C communication data length*/
    uint8_t              *pBuf;           /*!< I2C communication data buffer pointer*/
    __IO uint32_t         u32DataIndex;   /*!< I2C communication data transfer index*/
    __IO stc_i2c_com_status_t  enComStatus;    /*!< I2C communication status*/
} stc_i2c_communication_t;

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

/* Define port and pin for SDA and SCL */
#define I2C_SCL_PORT                    (GPIO_PORT_E)
#define I2C_SCL_PIN                     (GPIO_PIN_15)
#define I2C_SDA_PORT                    (GPIO_PORT_B)
#define I2C_SDA_PIN                     (GPIO_PIN_05)
#define I2C_GPIO_SCL_FUNC               (GPIO_FUNC_49)
#define I2C_GPIO_SDA_FUNC               (GPIO_FUNC_48)

#define I2C_EEI_IRQN_DEF                (INT001_IRQn)
#define I2C_RXI_IRQN_DEF                (INT002_IRQn)
#define I2C_TXI_IRQN_DEF                (INT003_IRQn)
#define I2C_TEI_IRQN_DEF                (INT004_IRQn)

#define I2C_INT_EEI_DEF                 (INT_SRC_I2C3_EEI)
#define I2C_INT_RXI_DEF                 (INT_SRC_I2C3_RXI)
#define I2C_INT_TXI_DEF                 (INT_SRC_I2C3_TXI)
#define I2C_INT_TEI_DEF                 (INT_SRC_I2C3_TEI)

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
static void BufWrite(uint8_t au8Buf[], uint8_t u8Data);
static uint8_t BufRead(uint8_t au8Buf[]);
static void Master_Start(void);
void Master_SendAddr(uint8_t u8Addr, uint8_t u8Dir);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t u8TxBuf[TEST_DATA_LEN];
static uint8_t u8RxBuf[TEST_DATA_LEN];
static stc_i2c_communication_t stcI2cCom;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Master send data kick start via interrupt .
 * @param  [in] au8Data          Data array
 * @param  [in] u32Size          Data size
 * @retval int32_t:
 *            - LL_OK:           Success
 *            - LL_ERR_BUSY:     Busy
 */
static int32_t I2C_Master_Transmit_IT(uint8_t au8Data[], uint32_t u32Size)
{
    int32_t i32Ret = LL_OK;

    if (I2C_COM_IDLE == stcI2cCom.enComStatus) {
        stcI2cCom.enComStatus = I2C_COM_BUSY;

        stcI2cCom.u32DataIndex = 0U;
        stcI2cCom.enMode = MD_TX;
        stcI2cCom.u32Len = u32Size;
        stcI2cCom.pBuf = au8Data;

        /* General start condition */
        Master_Start();
    } else {
        i32Ret = LL_ERR_BUSY;
    }
    return i32Ret;
}

/**
 * @brief  Master Rev data kick start via interrupt .
 * @param  [in] au8Data          Data array
 * @param  [in] u32Size          Data size
 * @retval int32_t:
 *            - LL_OK:           Success
 *            - LL_ERR_BUSY:     Busy
 */
static int32_t I2C_Master_Receive_IT(uint8_t au8Data[], uint32_t u32Size)
{
    int32_t i32Ret = LL_OK;

    if (I2C_COM_IDLE == stcI2cCom.enComStatus) {
        stcI2cCom.enComStatus = I2C_COM_BUSY;
        stcI2cCom.u32DataIndex = 0U;
        stcI2cCom.enMode = MD_RX;
        stcI2cCom.u32Len = u32Size;
        stcI2cCom.pBuf = au8Data;

        /* General start condition */
        Master_Start();
    } else {
        i32Ret = LL_ERR_BUSY;
    }
    return i32Ret;
}

/**
 * @brief   I2C EEI(communication error or event) interrupt callback function
 * @param   None
 * @retval  None
 */
static void I2C_EEI_Callback(void)
{
    /* If starf flag valid */
    if (SET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_START)) {
        I2C_ClearStatus(I2C_UNIT, I2C_CLR_STARTFCLR | I2C_CLR_NACKFCLR);
        I2C_IntCmd(I2C_UNIT, I2C_INT_STOP | I2C_INT_NACK, ENABLE);

        if (MD_TX == stcI2cCom.enMode) {
            /* Enable TEI interrupt which indicate address transfer end */
            I2C_IntCmd(I2C_UNIT, I2C_INT_TX_CPLT, ENABLE);

            Master_SendAddr(DEVICE_ADDR, I2C_DIR_TX);
        } else {
            /* if read data length is 1 */
            if (stcI2cCom.u32Len == 1U) {
                I2C_AckConfig(I2C_UNIT, I2C_NACK);
            }

            /* Enable RXI interrupt which indicate data receive buffer full */
            I2C_IntCmd(I2C_UNIT, I2C_INT_RX_FULL, ENABLE);

            Master_SendAddr(DEVICE_ADDR, I2C_DIR_RX);
        }
    }

    /* If NACK interrupt occurred */
    if (SET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_NACKF)) {
        /* clear NACK flag*/
        I2C_ClearStatus(I2C_UNIT, I2C_CLR_NACKFCLR);
        /* Stop tx or rx process*/
        I2C_IntCmd(I2C_UNIT, I2C_INT_TX_EMPTY | I2C_INT_RX_FULL | I2C_INT_TX_CPLT | I2C_INT_NACK, DISABLE);

        /* Generate stop condition */
        I2C_GenerateStop(I2C_UNIT);
    }

    if (SET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_STOP)) {
        /* Disable Stop flag interrupt */
        I2C_IntCmd(I2C_UNIT, I2C_INT_TX_EMPTY | I2C_INT_RX_FULL | I2C_INT_TX_CPLT | I2C_INT_STOP | I2C_INT_NACK, \
                   DISABLE);
        I2C_ClearStatus(I2C_UNIT, I2C_CLR_STOPFCLR);
        I2C_Cmd(I2C_UNIT, DISABLE);

        /* Communication finished */
        stcI2cCom.enComStatus = I2C_COM_IDLE;
    }
}

/**
 * @brief   I2C TXI(transfer buffer empty) interrupt callback function
 * @param   None
 * @retval  None
 */
static void I2C_TXI_Callback(void)
{
    if (stcI2cCom.u32DataIndex <= (stcI2cCom.u32Len - 1U)) {
        I2C_WriteData(I2C_UNIT, BufRead(stcI2cCom.pBuf));
    } else {
        I2C_IntCmd(I2C_UNIT, I2C_INT_TX_EMPTY, DISABLE);
        I2C_IntCmd(I2C_UNIT, I2C_INT_TX_CPLT, ENABLE);
    }
}

/**
 * @brief   I2C RXI(receive buffer full) interrupt callback function
 * @param   None
 * @retval  None
 */
static void I2C_RXI_Callback(void)
{
    if ((stcI2cCom.u32Len >= 2U) && (stcI2cCom.u32DataIndex == (stcI2cCom.u32Len - 2U))) {
        I2C_AckConfig(I2C_UNIT, I2C_NACK);
    }

    if (stcI2cCom.u32DataIndex == (stcI2cCom.u32Len - 1U)) {
        /* Enable Stop flag interrupt */
        I2C_IntCmd(I2C_UNIT, I2C_INT_STOP, ENABLE);
        /* Disable RXI interrupt */
        I2C_IntCmd(I2C_UNIT, I2C_INT_RX_FULL, DISABLE);
        /* Generate stop condition */
        I2C_GenerateStop(I2C_UNIT);

        I2C_AckConfig(I2C_UNIT, I2C_ACK);
    }

    BufWrite(stcI2cCom.pBuf, I2C_ReadData(I2C_UNIT));
}

/**
 * @brief   I2C TEI(Transfer end) interrupt callback function
 * @param   None
 * @retval  None
 */
static void I2C_TEI_Callback(void)
{
    I2C_IntCmd(I2C_UNIT, I2C_INT_TX_CPLT, DISABLE);

    if (stcI2cCom.u32DataIndex == 0U) {
        /* Indicate address send finished */
        if (SET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_TRA)) {
            /* If Address send receive ACK */
            if (RESET == I2C_GetStatus(I2C_UNIT, I2C_FLAG_NACKF)) {
                /* Config tx buffer empty interrupt function*/
                I2C_IntCmd(I2C_UNIT, I2C_INT_TX_EMPTY, ENABLE);
                /* Need transfer first data here */
                I2C_WriteData(I2C_UNIT, BufRead(stcI2cCom.pBuf));
            } else {
                I2C_IntCmd(I2C_UNIT, I2C_INT_NACK, DISABLE);
                /* Generate stop if receive NACK */
                I2C_IntCmd(I2C_UNIT, I2C_INT_STOP, ENABLE);
                /* Generate stop condition */
                I2C_GenerateStop(I2C_UNIT);
            }
        }
    } else {
        /* Data Send finish */
        I2C_IntCmd(I2C_UNIT, I2C_INT_STOP, ENABLE);
        /* Generate stop condition */
        I2C_GenerateStop(I2C_UNIT);
    }
}

/**
 * @brief   Initialize the I2C peripheral for master
 * @param   None
 * @retval int32_t:
 *            - LL_OK:              Success
 **           - LL_ERR_INVD_PARAM:  Invalid parameter
 */
static int32_t Master_Initialize(void)
{
    int32_t i32Ret;
    stc_i2c_init_t stcI2cInit;
    stc_irq_signin_config_t stcIrqRegCfg;
    float32_t fErr;

    I2C_DeInit(I2C_UNIT);

    stcI2cCom.enComStatus = I2C_COM_IDLE;

    (void)I2C_StructInit(&stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV2;
    stcI2cInit.u32Baudrate = I2C_BAUDRATE;
    stcI2cInit.u32SclTime = 3U;
    i32Ret = I2C_Init(I2C_UNIT, &stcI2cInit, &fErr);
    if (LL_OK == i32Ret) {
        stcIrqRegCfg.enIRQn = I2C_EEI_IRQN_DEF;
        stcIrqRegCfg.enIntSrc = I2C_INT_EEI_DEF;
        stcIrqRegCfg.pfnCallback = &I2C_EEI_Callback;
        (void)INTC_IrqSignIn(&stcIrqRegCfg);
        NVIC_ClearPendingIRQ(stcIrqRegCfg.enIRQn);
        NVIC_SetPriority(stcIrqRegCfg.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqRegCfg.enIRQn);

        stcIrqRegCfg.enIRQn = I2C_RXI_IRQN_DEF;
        stcIrqRegCfg.enIntSrc = I2C_INT_RXI_DEF;
        stcIrqRegCfg.pfnCallback = &I2C_RXI_Callback;
        (void)INTC_IrqSignIn(&stcIrqRegCfg);
        NVIC_ClearPendingIRQ(stcIrqRegCfg.enIRQn);
        NVIC_SetPriority(stcIrqRegCfg.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqRegCfg.enIRQn);

        stcIrqRegCfg.enIRQn = I2C_TEI_IRQN_DEF;
        stcIrqRegCfg.enIntSrc = I2C_INT_TEI_DEF;
        stcIrqRegCfg.pfnCallback = &I2C_TEI_Callback;
        (void)INTC_IrqSignIn(&stcIrqRegCfg);
        NVIC_ClearPendingIRQ(stcIrqRegCfg.enIRQn);
        NVIC_SetPriority(stcIrqRegCfg.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqRegCfg.enIRQn);

        stcIrqRegCfg.enIRQn = I2C_TXI_IRQN_DEF;
        stcIrqRegCfg.enIntSrc = I2C_INT_TXI_DEF;
        stcIrqRegCfg.pfnCallback = &I2C_TXI_Callback;
        (void)INTC_IrqSignIn(&stcIrqRegCfg);
        NVIC_ClearPendingIRQ(stcIrqRegCfg.enIRQn);
        NVIC_SetPriority(stcIrqRegCfg.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqRegCfg.enIRQn);

        I2C_BusWaitCmd(I2C_UNIT, ENABLE);
    }
    return i32Ret;
}

/**
 * @brief  Send Start condition.
 * @param  None
 * @retval None
 */
static void Master_Start(void)
{
    /* I2C function command */
    I2C_Cmd(I2C_UNIT, ENABLE);
    /* Config startf and slave address match interrupt function*/
    I2C_IntCmd(I2C_UNIT, I2C_INT_START, ENABLE);

    I2C_SWResetCmd(I2C_UNIT, ENABLE);
    I2C_SWResetCmd(I2C_UNIT, DISABLE);
    /* generate start signal */
    I2C_GenerateStart(I2C_UNIT);
}

/**
 * @brief  Send slave address
 **
 * @param  [in] u8Addr            The slave address
 * @param  [in] enDir             Can be cmp_normal_int or I2C_DIR_RX
 * @retval None
 */
void Master_SendAddr(uint8_t u8Addr, uint8_t u8Dir)
{
    /* Send I2C address */
    I2C_WriteData(I2C_UNIT, (u8Addr << 1U) | u8Dir);
}

/**
 * @brief   static function for buffer write.
 * @param   [in] au8Buf     Buffer pointer
 * @param   [in] u8Data     The data to be write.
 * @retval  None
 */
static void BufWrite(uint8_t au8Buf[], uint8_t u8Data)
{
    if (stcI2cCom.u32DataIndex >= stcI2cCom.u32Len) {
        /* error */
        for (;;) {
            ;
        }
    }
    au8Buf[stcI2cCom.u32DataIndex] = u8Data;
    stcI2cCom.u32DataIndex++;
}

/**
 * @brief   Static function for buffer read.
 * @param   [in] au8Buf[]   Buffer pointer
 * @retval  uint8_t  The data read out from buffer.
 */
static uint8_t BufRead(uint8_t au8Buf[])
{
    uint8_t temp;
    if (stcI2cCom.u32DataIndex >= stcI2cCom.u32Len) {
        /* error */
        for (;;) {
            ;
        }
    }
    temp = au8Buf[stcI2cCom.u32DataIndex];
    stcI2cCom.u32DataIndex++;

    return temp;
}

/**
 * @brief  Main function
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t  main(void)
{
    uint32_t i;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* BSP initialization */
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();

    for (i = 0U; i < TEST_DATA_LEN; i++) {
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
        /* Initialize error*/
        BSP_LED_Toggle(LED_RED);
        for (;;) {
            ;
        }
    }

    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    /* Wait key */
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_2)) {
        ;
    }

    /* I2C master data write */
    while (LL_OK != I2C_Master_Transmit_IT(u8TxBuf, TEST_DATA_LEN)) {
        ;
    }

    /* Wait communication finished*/
    while (I2C_COM_BUSY == stcI2cCom.enComStatus) {
        if (TIMEOUT == i) {
            /* Communication time out*/
            BSP_LED_Toggle(LED_RED);
            for (;;) {
                ;
            }
        }
        i++;
    }

    /* 5mS delay for device*/
    DDL_DelayMS(500UL);

    while (LL_OK != I2C_Master_Receive_IT(u8RxBuf, TEST_DATA_LEN)) {
        ;
    }

    i = 0UL;
    /* Wait communicaiton finished*/
    while (I2C_COM_BUSY == stcI2cCom.enComStatus) {
        if (TIMEOUT == i) {
            /* Communication time out*/
            BSP_LED_Toggle(LED_RED);
            for (;;) {
                ;
            }
        }
        i++;
    }

    /* Compare the data */
    for (i = 0U; i < TEST_DATA_LEN; i++) {
        if (u8TxBuf[i] != u8RxBuf[i]) {
            /* Data write error*/
            BSP_LED_Toggle(LED_RED);
            for (;;) {
                BSP_LED_Toggle(LED_RED);
                DDL_DelayMS(500UL);
            }
        }
    }

    /* Communication finished */
    for (;;) {
        BSP_LED_Toggle(LED_BLUE);
        DDL_DelayMS(500UL);
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
