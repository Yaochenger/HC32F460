/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file main.c
 **
 ** \brief I2C slave DMA sample
 **
 **   - 2021-09-10  CDT First version for Device Driver Library of I2C
 **     Slave DMA example
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Define I2C unit used for the example */
#define I2C_UNIT                        (M4_I2C3)
/* Define slave device address for example */
#define SLAVE_ADDRESS                   (0x06u)
//#define I2C_10BITS_ADDRESS              (1u)

/* Define port and pin for SDA and SCL */
#define I2C_SCL_PORT                    (PortE)
#define I2C_SCL_PIN                     (Pin15)
#define I2C_SDA_PORT                    (PortB)
#define I2C_SDA_PIN                     (Pin05)
#define I2C_GPIO_SCL_FUNC               (Func_I2c3_Scl)
#define I2C_GPIO_SDA_FUNC               (Func_I2c3_Sda)

#define I2C_DMA_TX_EVT                  (EVT_I2C3_TEI)
#define I2C_DMA_RX_EVT                  (EVT_I2C3_RXI)

#define I2C_FCG_USE                     (PWC_FCG1_PERIPH_I2C3)

#define TIMEOUT                         (0x10000ul)

/* Define Write and read data length for the example */
#define TEST_DATA_LEN                   (256u)
/* Define i2c baudrate */
#define I2C_BAUDRATE                    (400000ul)

#define I2C_DMA_UNIT                    (M4_DMA1)
#define I2C_TX_DMA_CH                   (DmaCh0)
#define I2C_RX_DMA_CH                   (DmaCh1)

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

static void I2C_Tx_DmaInit(void)
{
    stc_dma_config_t stcDmaCfg;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcDmaCfg);

    /* Configure TX DMA */
    stcDmaCfg.u16BlockSize = 1u;
    stcDmaCfg.u16TransferCnt = 0u;;
    stcDmaCfg.u32SrcAddr = 0u;
    stcDmaCfg.u32DesAddr = (uint32_t)(&I2C_UNIT->DTR);
    stcDmaCfg.stcDmaChCfg.enSrcInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enDesInc = AddressFix;
    stcDmaCfg.stcDmaChCfg.enTrnWidth = Dma8Bit;
    stcDmaCfg.stcDmaChCfg.enIntEn = Disable;
    DMA_InitChannel(I2C_DMA_UNIT, I2C_TX_DMA_CH, &stcDmaCfg);

    DMA_SetTriggerSrc(I2C_DMA_UNIT, I2C_TX_DMA_CH, I2C_DMA_TX_EVT);

    /* Enable DMA */
    DMA_Cmd(I2C_DMA_UNIT, Enable);
}

static void I2C_Rx_DmaInit(void)
{
    stc_dma_config_t stcDmaCfg;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcDmaCfg);

    /* Configure TX DMA */
    stcDmaCfg.u16BlockSize = 1u;
    stcDmaCfg.u16TransferCnt = 0u;;
    stcDmaCfg.u32SrcAddr = (uint32_t)(&I2C_UNIT->DRR);
    stcDmaCfg.u32DesAddr = 0u;
    stcDmaCfg.stcDmaChCfg.enSrcInc = AddressFix;
    stcDmaCfg.stcDmaChCfg.enDesInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enTrnWidth = Dma8Bit;
    stcDmaCfg.stcDmaChCfg.enIntEn = Disable;
    DMA_InitChannel(I2C_DMA_UNIT, I2C_RX_DMA_CH, &stcDmaCfg);

    DMA_SetTriggerSrc(I2C_DMA_UNIT, I2C_RX_DMA_CH, I2C_DMA_RX_EVT);

    /* Enable DMA */
    DMA_Cmd(I2C_DMA_UNIT, Enable);
}

static void I2C_Tx_DmaStart(uint8_t *pTxBuf, uint16_t u16Cnt)
{
    DMA_SetSrcAddress(I2C_DMA_UNIT, I2C_TX_DMA_CH, (uint32_t)pTxBuf);
    DMA_SetTransferCnt(I2C_DMA_UNIT, I2C_TX_DMA_CH, u16Cnt);

    /* Enable DMA CH */
    DMA_ChannelCmd(I2C_DMA_UNIT, I2C_TX_DMA_CH, Enable);
}

static void I2C_Rx_DmaStart(uint8_t *pRxBuf, uint16_t u16Cnt)
{
    DMA_SetDesAddress(I2C_DMA_UNIT, I2C_RX_DMA_CH, (uint32_t)pRxBuf);
    DMA_SetTransferCnt(I2C_DMA_UNIT, I2C_RX_DMA_CH, u16Cnt);

    /* Enable DMA CH */
    DMA_ChannelCmd(I2C_DMA_UNIT, I2C_RX_DMA_CH, Enable);
}

/**
 ******************************************************************************
 ** \brief  Slave receive data
 **
 ** \param  pu8RxData             Pointer to the data buffer
 ** \param  u16Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - Error:              Failed
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
static en_result_t I2C_Slave_Receive_DMA(uint8_t *pu8RxData, uint16_t u16Size, uint32_t u32TimeOut)
{
    en_result_t enRet;
    /* clear all status */

    I2C_Cmd(I2C_UNIT, Enable);

    /* Clear status */
    I2C_ClearStatus(I2C_UNIT,I2C_CLR_STOPFCLR | I2C_CLR_NACKFCLR );

    /* Wait slave address matched */
    while(Reset == I2C_GetStatus(I2C_UNIT, I2C_SR_SLADDR0F))
    {
        ;
    }
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_SLADDR0FCLR);

    if(Reset == I2C_GetStatus(I2C_UNIT, I2C_SR_TRA))
    {
        /* Slave receive data*/
        I2C_Rx_DmaStart(pu8RxData, u16Size);
        /* Wait for DMA complete */
        while(Set != DMA_GetIrqFlag(I2C_DMA_UNIT,I2C_RX_DMA_CH, TrnCpltIrq))
        {
            ;
        }
        DMA_ClearIrqFlag(I2C_DMA_UNIT, I2C_RX_DMA_CH, TrnCpltIrq);
        /* Wait stop condition */
        enRet = I2C_WaitStatus(I2C_UNIT, I2C_SR_STOPF, Set, u32TimeOut);
    }
    else
    {
        enRet = Error;
    }

    I2C_Cmd(I2C_UNIT, Disable);
    return enRet;
}

/**
 ******************************************************************************
 ** \brief  Slave transmit data
 **
 ** \param  pu8TxData             Pointer to the data buffer
 ** \param  u16Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - Error:              Failed
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
static en_result_t I2C_Slave_Transmit_DMA(uint8_t *pu8TxData, uint16_t u16Size, uint32_t u32TimeOut)
{
    en_result_t enRet;

    I2C_Cmd(I2C_UNIT, Enable);

    /* Clear status */
    I2C_ClearStatus(I2C_UNIT,I2C_CLR_STOPFCLR | I2C_CLR_NACKFCLR );

    /* Wait slave address matched */
    while(Reset == I2C_GetStatus(I2C_UNIT, I2C_SR_SLADDR0F))
    {
        ;
    }
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_SLADDR0FCLR);

#ifdef I2C_10BITS_ADDRESS
    /* Wait slave address matched */
    while(Reset == I2C_GetStatus(I2C_UNIT, I2C_SR_SLADDR0F))
    {
        ;
    }
    I2C_ClearStatus(I2C_UNIT, I2C_CLR_SLADDR0FCLR);
#endif

    if(Reset == I2C_GetStatus(I2C_UNIT, I2C_SR_TRA))
    {
        enRet = Error;
    }
    else
    {
        I2C_Tx_DmaStart(pu8TxData + 1u, (u16Size - 1u));
        I2C_WriteData(I2C_UNIT, pu8TxData[0]);
        
        /* Wait for DMA complete */
        while(Set != DMA_GetIrqFlag(I2C_DMA_UNIT,I2C_TX_DMA_CH, TrnCpltIrq))
        {
            ;
        }
        DMA_ClearIrqFlag(I2C_DMA_UNIT, I2C_TX_DMA_CH, TrnCpltIrq);
        
        enRet = I2C_WaitStatus(I2C_UNIT, I2C_SR_TENDF, Set, u32TimeOut);
        
        /* Release SCL pin */
        (void)I2C_ReadData(I2C_UNIT);
        /* Wait stop condition */
        enRet = I2C_WaitStatus(I2C_UNIT, I2C_SR_STOPF, Set, u32TimeOut);
    }

    I2C_Cmd(I2C_UNIT, Disable);
    return enRet;
}

/**
 ******************************************************************************
 ** \brief  Initialize the I2C peripheral for slave
 ** \param  None
 ** \return Process result
 **         - Ok:                     Success
 **         - ErrorInvalidParameter:  Invalid parameter
 ******************************************************************************/
static en_result_t Slave_Initialize(void)
{
    en_result_t enRet;
    stc_i2c_init_t stcI2cInit;
    float32_t fErr;

    I2C_DeInit(I2C_UNIT);

    MEM_ZERO_STRUCT(stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV2;
    stcI2cInit.u32Baudrate = I2C_BAUDRATE;
    stcI2cInit.u32SclTime = 0ul;
    enRet = I2C_Init(I2C_UNIT, &stcI2cInit, &fErr);

    if(Ok == enRet)
    {
         /* Set slave address */
#ifdef I2C_10BITS_ADDRESS
        I2C_SlaveAdr0Config(I2C_UNIT, Enable, Adr10bit, SLAVE_ADDRESS);
#else
        I2C_SlaveAdr0Config(I2C_UNIT, Enable, Adr7bit, SLAVE_ADDRESS);
#endif
    }
    return enRet;
}

/**
 *******************************************************************************
 ** \brief  Main function of template project
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 **
 ******************************************************************************/
int32_t  main(void)
{
    uint8_t u8RxBuf[TEST_DATA_LEN];

    memset(u8RxBuf, 0x00, TEST_DATA_LEN);

    /* BSP initialization */
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();

    /* Initialize I2C port*/
    PORT_SetFunc(I2C_SCL_PORT, I2C_SCL_PIN, I2C_GPIO_SCL_FUNC, Disable);
    PORT_SetFunc(I2C_SDA_PORT, I2C_SDA_PIN, I2C_GPIO_SDA_FUNC, Disable);

    /* Enable I2C Peripheral*/
    PWC_Fcg1PeriphClockCmd(I2C_FCG_USE, Enable);
    /* Configuration peripheral clock for DMA */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA1, Enable);
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);
    
    /* Initialize I2C peripheral and enable function*/
    Slave_Initialize();
    I2C_Tx_DmaInit();
    I2C_Rx_DmaInit();
    
    while(1)
    {
        if(Ok == I2C_Slave_Receive_DMA(u8RxBuf, TEST_DATA_LEN, TIMEOUT))
        {
            if(Ok != I2C_Slave_Transmit_DMA(u8RxBuf, TEST_DATA_LEN, TIMEOUT))
            {
                /* Failed */
                break;
            }
        }
        else
        {
            /* Failed */
            break;
        }
    }

    /* Failed */
    while(1)
    {
        BSP_LED_Toggle(LED_RED);
        Ddl_Delay1ms(500ul);
    }

}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
