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
 ** \brief I2C master DMA sample
 **
 **   - 2021-09-10  CDT First version for Device Driver Library of I2C
 **     master DMA example
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
#define I2C_UNIT                        (M4_I2C1)
/* Define slave device address for example */
#define DEVICE_ADDRESS                  (0x06u)
//#define I2C_10BITS_ADDRESS              (1u)

/* Define port and pin for SDA and SCL */
#define I2C_SCL_PORT                    (PortC)
#define I2C_SCL_PIN                     (Pin04)
#define I2C_SDA_PORT                    (PortC)
#define I2C_SDA_PIN                     (Pin05)
#define I2C_GPIO_SCL_FUNC               (Func_I2c1_Scl)
#define I2C_GPIO_SDA_FUNC               (Func_I2c1_Sda)

#define I2C_DMA_TX_EVT                  (EVT_I2C1_TEI)
#define I2C_DMA_RX_EVT                  (EVT_I2C1_RXI)

#define I2C_FCG_USE                     (PWC_FCG1_PERIPH_I2C1)

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
    stcDmaCfg.u32DesAddr = 0u;
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
    stcDmaCfg.u32SrcAddr = 0u;
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
    DMA_SetDesAddress(I2C_DMA_UNIT, I2C_TX_DMA_CH, (uint32_t)(&I2C_UNIT->DTR));
    DMA_SetTransferCnt(I2C_DMA_UNIT, I2C_TX_DMA_CH, u16Cnt);

    /* Enable DMA CH */
    DMA_ChannelCmd(I2C_DMA_UNIT, I2C_TX_DMA_CH, Enable);
}

static void I2C_Rx_DmaStart(uint8_t *pRxBuf, uint16_t u16Cnt)
{
    DMA_SetSrcAddress(I2C_DMA_UNIT, I2C_RX_DMA_CH, (uint32_t)(&I2C_UNIT->DRR));
    DMA_SetDesAddress(I2C_DMA_UNIT, I2C_RX_DMA_CH, (uint32_t)pRxBuf);
    DMA_SetTransferCnt(I2C_DMA_UNIT, I2C_RX_DMA_CH, u16Cnt);

    /* Enable DMA CH */
    DMA_ChannelCmd(I2C_DMA_UNIT, I2C_RX_DMA_CH, Enable);
}

/**
 ******************************************************************************
 ** \brief  Master transmit data DMA
 **
 ** \param  u16DevAddr            The slave address
 ** \param  pu8TxData             Pointer to the data buffer
 ** \param  u16Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
static en_result_t I2C_Master_Transmit_DMA(uint16_t u16DevAddr, uint8_t *pu8TxData, uint16_t u16Size, uint32_t u32TimeOut)
{
    en_result_t enRet;
    I2C_Cmd(I2C_UNIT, Enable);

    I2C_SoftwareResetCmd(I2C_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_UNIT, Disable);
    enRet = I2C_Start(I2C_UNIT,u32TimeOut);
    if(Ok == enRet)
    {

#ifdef I2C_10BITS_ADDRESS
        enRet = I2C_Trans10BitAddr(I2C_UNIT, u16DevAddr, I2CDirTrans, u32TimeOut);
#else
        enRet = I2C_TransAddr(I2C_UNIT, (uint8_t)u16DevAddr, I2CDirTrans, u32TimeOut);
#endif

        if(Ok == enRet)
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
        }
    }

    I2C_Stop(I2C_UNIT,u32TimeOut);
    I2C_Cmd(I2C_UNIT, Disable);

    return enRet;
}

/**
 ******************************************************************************
 ** \brief  Master receive data DMA
 **
 ** \param  u16DevAddr            The slave address
 ** \param  pu8RxData             Pointer to the data buffer
 ** \param  u16Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
static en_result_t I2C_Master_Receive_DMA(uint16_t u16DevAddr, uint8_t *pu8RxData, uint16_t u16Size, uint32_t u32TimeOut)
{
    en_result_t enRet;

    I2C_Cmd(I2C_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_UNIT, Disable);
    enRet = I2C_Start(I2C_UNIT,u32TimeOut);
    if(Ok == enRet)
    {
        if(1ul == u16Size)
        {
            I2C_AckConfig(I2C_UNIT, I2c_NACK);
        }
        
#ifdef I2C_10BITS_ADDRESS
        enRet = I2C_Trans10BitAddr(I2C_UNIT, u16DevAddr, I2CDirReceive, u32TimeOut);
#else
        enRet = I2C_TransAddr(I2C_UNIT, (uint8_t)u16DevAddr, I2CDirReceive, u32TimeOut);
#endif

        if(Ok == enRet)
        {
            if(u16Size <= 2u)
            {
                enRet = I2C_MasterDataReceiveAndStop(I2C_UNIT, pu8RxData, u16Size, u32TimeOut);
            }
            else
            {
                I2C_Rx_DmaStart(pu8RxData, u16Size - 2u);
                /* Wait for DMA complete */
                while(Set != DMA_GetIrqFlag(I2C_DMA_UNIT,I2C_RX_DMA_CH, TrnCpltIrq))
                {
                    ;
                }
                DMA_ClearIrqFlag(I2C_DMA_UNIT, I2C_RX_DMA_CH, TrnCpltIrq);
                enRet = I2C_WaitStatus(I2C_UNIT, I2C_SR_RFULLF, Set, u32TimeOut);
                I2C_AckConfig(I2C_UNIT, I2c_NACK);
                pu8RxData[u16Size - 2u] = I2C_ReadData(I2C_UNIT);
                enRet = I2C_WaitStatus(I2C_UNIT, I2C_SR_RFULLF, Set, u32TimeOut);
                I2C_ClearStatus(I2C_UNIT, I2C_CLR_STOPFCLR);
                I2C_GenerateStop(I2C_UNIT, Enable);
                pu8RxData[u16Size - 1u] = I2C_ReadData(I2C_UNIT);
            }
        }

        I2C_AckConfig(I2C_UNIT, I2c_ACK);
    }

    if(Ok != enRet)
    {
        I2C_Stop(I2C_UNIT,u32TimeOut);
    }
    enRet = I2C_WaitStatus(I2C_UNIT, I2C_SR_STOPF, Set, u32TimeOut);
    I2C_Cmd(I2C_UNIT, Disable);
    return enRet;
}

/**
 ******************************************************************************
 ** \brief   Initialize the I2C peripheral for master
 ** \param   None
 ** \retval en_result_t                Enumeration value:
 **          - Ok:                     Success
 **          - ErrorInvalidParameter:  Invalid parameter
 ******************************************************************************/
static en_result_t Master_Initialize(void)
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

    I2C_BusWaitCmd(I2C_UNIT, Enable);

    return enRet;
}

/**
 *******************************************************************************
 ** \brief  Main function of template project
 ** \param  None
 ** \retval int32_t return value, if needed
 ******************************************************************************/
int32_t  main(void)
{
    uint8_t u8TxBuf[TEST_DATA_LEN];
    uint8_t u8RxBuf[TEST_DATA_LEN];
    uint32_t i;

    for(i=0ul; i<TEST_DATA_LEN; i++)
    {
        u8TxBuf[i] = (uint8_t)(i+1ul);
    }
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
    Master_Initialize();

    I2C_Tx_DmaInit();
    I2C_Rx_DmaInit();

    /* Wait key */
    while (Reset == BSP_KEY_GetStatus(BSP_KEY_2))
    {
        ;
    }

    Ddl_Delay1ms(5ul);
    
    I2C_Master_Transmit_DMA(DEVICE_ADDRESS, u8TxBuf, TEST_DATA_LEN, TIMEOUT);

    /* 50mS delay for device*/
    Ddl_Delay1ms(50ul);

    I2C_Master_Receive_DMA(DEVICE_ADDRESS, u8RxBuf, TEST_DATA_LEN, TIMEOUT);

    /* Compare the data */
    for(i=0ul; i<TEST_DATA_LEN; i++)
    {
        if(u8TxBuf[i] != u8RxBuf[i])
        {
            /* Data write error*/
            while(1)
            {
                BSP_LED_Toggle(LED_RED);
                Ddl_Delay1ms(500ul);
            }
        }
    }
    
    /* I2C master comunication success */
    while(1)
    {
        BSP_LED_Toggle(LED_GREEN);
        Ddl_Delay1ms(500ul);
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
