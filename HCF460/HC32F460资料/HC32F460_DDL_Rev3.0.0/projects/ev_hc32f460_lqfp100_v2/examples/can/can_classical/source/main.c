/**
 *******************************************************************************
 * @file  can/can_classical/source/main.c
 * @brief Main program of CAN classical for the Device Driver Library.
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
 * @addtogroup CAN_Classical
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* CAN unit definitions. */
#define CAN_UNIT                        (CM_CAN)
#define CAN_PERIPH_CLK                  (FCG1_PERIPH_CAN)

#define CAN_TX_PORT                     (GPIO_PORT_B)
#define CAN_TX_PIN                      (GPIO_PIN_07)
#define CAN_TX_PIN_FUNC                 (GPIO_FUNC_50)

#define CAN_RX_PORT                     (GPIO_PORT_B)
#define CAN_RX_PIN                      (GPIO_PIN_06)
#define CAN_RX_PIN_FUNC                 (GPIO_FUNC_51)

#define CAN_INT_PRIO                    (DDL_IRQ_PRIO_03)
#define CAN_INT_SRC                     (INT_SRC_CAN_INT)
#define CAN_INT_IRQn                    (INT122_IRQn)

/* CAN interrupt type selection. */
#define CAN_INT_SEL                     (CAN_INT_STB_TX      | \
                                         CAN_INT_PTB_TX      | \
                                         CAN_INT_RX_BUF_WARN | \
                                         CAN_INT_RX_BUF_FULL | \
                                         CAN_INT_RX_OVERRUN  | \
                                         CAN_INT_RX)

/* CAN PHY standby pin. */
#define CAN_PHY_STBY_PORT                (GPIO_PORT_D)
#define CAN_PHY_STBY_PIN                 (GPIO_PIN_15)

/* Acceptance filter. */
#define CAN_FILTER_SEL                  (CAN_FILTER1 | CAN_FILTER2 | CAN_FILTER3)
#define CAN_FILTER_NUM                  (3U)

#define CAN_FILTER1_ID                  (0x701UL)
#define CAN_FILTER1_ID_MASK             (0x0UL)
#define CAN_FILTER1_ID_TYPE             (CAN_ID_STD)        /*!< Only accept frames with standard ID 0x701. */

#define CAN_FILTER2_ID                  (0x12131415UL)
#define CAN_FILTER2_ID_MASK             (0x000000F0UL)
#define CAN_FILTER2_ID_TYPE             (CAN_ID_EXT)        /*!< Accept frames with extended ID 0x121314x5. */

#define CAN_FILTER3_ID                  (0x1A1B1C1DUL)
#define CAN_FILTER3_ID_MASK             (0x0000000FUL)
#define CAN_FILTER3_ID_TYPE             (CAN_ID_STD_EXT)    /*!< Accept frames with extended ID 0x1A1B1C1x, \
                                                                 standard ID 0x41x(0x1A1B1C1D & 0x7FF(standard ID mask)). */
/* Message ID definitions. */
#define CAN_TX_ID1                      (0x601UL)
#define CAN_TX_ID1_IDE                  (0U)

#define CAN_TX_ID2                      (0x602UL)
#define CAN_TX_ID2_IDE                  (0U)

#define CAN_TX_ID3                      (0x1020301UL)
#define CAN_TX_ID3_IDE                  (1U)

#define CAN_TX_ID4                      (0x1030402UL)
#define CAN_TX_ID4_IDE                  (1U)

#define CAN_TX_ID5                      (0x1030503UL)
#define CAN_TX_ID5_IDE                  (1U)

/* Size of the TX data. */
#define CAN_TX_DLC                      (CAN_DLC8)
#define CAN_TX_DATA_SIZE                (8U)

/* Timeout value */
#define CAN_TX_TIMEOUT_MS               (100U)

/* TX process */
#define CAN_TX_PROCESS_PTB              (1U)
#define CAN_TX_PROCESS_STB              (2U)
#define CAN_TX_PROCESS_PTB_STB          (3U)

/* Number of RX frame */
#define CAN_RX_FRAME_NUM                (10U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void CanPinConfig(void);
static void CanInitConfig(void);
static void CanIrqConfig(void);
static void CanPhyEnable(void);

static void CanTx(void);
static void CanRx(void);

static void CAN_IrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
#if (LL_PRINT_ENABLE == DDL_ON)
const static char *m_s8IDTypeStr[] = {
    "standard",
    "extended",
};

const static char *m_s8FrameTypeStr[] = {
    "classical",
    "CAN-FD",
};

const static char *m_s8ErrorTypeStr[] = {
    "NO error.",
    "Bit Error.",
    "Form Error.",
    "Stuff Error.",
    "ACK Error.",
    "CRC Error.",
    "Other Error.",
    "Error type is NOT defined.",
};
#endif

static stc_can_rx_frame_t m_astRxFrame[CAN_RX_FRAME_NUM];

static __IO uint8_t m_u8RxFlag    = 0U;
static __IO uint8_t m_u8PTBTxFlag = 0U;
static __IO uint8_t m_u8STBTxFlag = 0U;
static __IO uint8_t m_u8TxStart   = 0U;
static __IO uint8_t m_u8TxProcess = 0U;
static __IO uint32_t m_u32TxTick  = 0UL;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of can_classical project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);
    /* Configures the system clock to 200MHz. */
    BSP_CLK_Init();
    /* BSP keys are used as TX triggers. */
    BSP_KEY_Init();
    /* Initializes UART for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configures CAN. */
    CanPinConfig();
    CanInitConfig();
    CanIrqConfig();
    CanPhyEnable();
    /* For CAN TX timeout. */
    (void)SysTick_Init(1000U);
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /***************** Configuration end, application start **************/
    for (;;) {
        if (m_u8RxFlag != 0U) {
            /* Read frames here or in CAN_IrqCallback */
            CanRx();
            m_u8RxFlag = 0U;
        }
        CanTx();
    }
}

/**
 * @brief  Specifies pin funtion for TXD and RXD.
 * @param  None
 * @retval None
 */
static void CanPinConfig(void)
{
    GPIO_SetFunc(CAN_TX_PORT, CAN_TX_PIN, CAN_TX_PIN_FUNC);
    GPIO_SetFunc(CAN_RX_PORT, CAN_RX_PIN, CAN_RX_PIN_FUNC);
}

/**
 * @brief  CAN initial configuration.
 * @param  None
 * @retval None
 */
static void CanInitConfig(void)
{
    stc_can_init_t stcCanInit;
    stc_can_filter_config_t astcFilter[CAN_FILTER_NUM] = {
        {CAN_FILTER1_ID, CAN_FILTER1_ID_MASK, CAN_FILTER1_ID_TYPE},
        {CAN_FILTER2_ID, CAN_FILTER2_ID_MASK, CAN_FILTER2_ID_TYPE},
        {CAN_FILTER3_ID, CAN_FILTER3_ID_MASK, CAN_FILTER3_ID_TYPE},
    };

    /* Initializes CAN. */
    (void)CAN_StructInit(&stcCanInit);
    stcCanInit.stcBitCfg.u32Prescaler = 1U;
    stcCanInit.stcBitCfg.u32TimeSeg1  = 6U;
    stcCanInit.stcBitCfg.u32TimeSeg2  = 2U;
    stcCanInit.stcBitCfg.u32SJW       = 2U;
    stcCanInit.pstcFilter             = astcFilter;
    stcCanInit.u16FilterSelect        = CAN_FILTER_SEL;
    stcCanInit.u8WorkMode             = CAN_WORK_MD_NORMAL;

    /* Enable peripheral clock of CAN. */
    FCG_Fcg1PeriphClockCmd(CAN_PERIPH_CLK, ENABLE);
    (void)CAN_Init(CAN_UNIT, &stcCanInit);
    /* Enable the interrupts, the status flags can be read. */
    CAN_IntCmd(CAN_UNIT, CAN_INT_ALL, DISABLE);
    /* Enalbe the interrupts that needed. */
    CAN_IntCmd(CAN_UNIT, CAN_INT_SEL, ENABLE);
}

/**
 * @brief  CAN interrupt configuration.
 * @param  None
 * @retval None
 */
static void CanIrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = CAN_INT_SRC;
    stcIrq.enIRQn      = CAN_INT_IRQn;
    stcIrq.pfnCallback = &CAN_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, CAN_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);
}

/**
 * @brief  Set CAN PHY STB pin as low.
 * @param  None
 * @retval None
 */
static void CanPhyEnable(void)
{
    /* Set PYH STB pin as low. */
    GPIO_ResetPins(CAN_PHY_STBY_PORT, CAN_PHY_STBY_PIN);
    GPIO_OutputCmd(CAN_PHY_STBY_PORT, CAN_PHY_STBY_PIN, ENABLE);
}

/**
 * @brief  CAN transmission.
 * @param  None
 * @retval None
 */
static void CanTx(void)
{
    uint8_t i;
    stc_can_tx_frame_t stcTx1;
    stc_can_tx_frame_t stcTx2;
    stc_can_tx_frame_t stcTx3;
    stc_can_tx_frame_t stcTx4;
    stc_can_tx_frame_t stcTx5;

    for (i = 0U; i < CAN_TX_DATA_SIZE; i++) {
        stcTx1.au8Data[i] = i + 0x10U;
        stcTx2.au8Data[i] = i + 0x20U;
        stcTx3.au8Data[i] = i + 0x30U;
        stcTx4.au8Data[i] = i + 0x40U;
        stcTx5.au8Data[i] = i + 0x50U;
    }

    stcTx1.u32Ctrl = 0x0UL;
    stcTx1.u32ID   = CAN_TX_ID1;
    stcTx1.IDE     = CAN_TX_ID1_IDE;
    stcTx1.DLC     = CAN_TX_DLC;

    stcTx2.u32Ctrl = 0x0UL;
    stcTx2.u32ID   = CAN_TX_ID2;
    stcTx2.IDE     = CAN_TX_ID2_IDE;
    stcTx2.DLC     = CAN_TX_DLC;

    stcTx3.u32Ctrl = 0x0UL;
    stcTx3.u32ID   = CAN_TX_ID3;
    stcTx3.IDE     = CAN_TX_ID3_IDE;
    stcTx3.DLC     = CAN_TX_DLC;

    stcTx4.u32Ctrl = 0x0UL;
    stcTx4.u32ID   = CAN_TX_ID4;
    stcTx4.IDE     = CAN_TX_ID4_IDE;
    stcTx4.DLC     = CAN_TX_DLC;

    stcTx5.u32Ctrl = 0x0UL;
    stcTx5.u32ID   = CAN_TX_ID5;
    stcTx5.IDE     = CAN_TX_ID5_IDE;
    stcTx5.DLC     = CAN_TX_DLC;

    if (BSP_KEY_GetStatus(BSP_KEY_1) == SET) {
        if (m_u8TxStart == 0U) {
            m_u8TxProcess++;
            if (m_u8TxProcess > CAN_TX_PROCESS_PTB_STB) {
                m_u8TxProcess = CAN_TX_PROCESS_PTB;
            }
            switch (m_u8TxProcess) {
                case CAN_TX_PROCESS_PTB:
                    /* Transmit one frame via PTB */
                    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_PTB, &stcTx1);
                    CAN_StartTx(CAN_UNIT, CAN_TX_REQ_PTB);
                    break;
                case CAN_TX_PROCESS_STB:
                    /* Transmit one frame via STB */
                    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_STB, &stcTx2);
                    CAN_StartTx(CAN_UNIT, CAN_TX_REQ_STB_ONE);
                    break;
                case CAN_TX_PROCESS_PTB_STB:
                default:
                    /* Transmit frames via PTB and STB */
                    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_PTB, &stcTx3);
                    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_STB, &stcTx4);
                    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_STB, &stcTx5);
                    CAN_StartTx(CAN_UNIT, CAN_TX_REQ_PTB);
                    CAN_StartTx(CAN_UNIT, CAN_TX_REQ_STB_ALL);
                    break;
            }
            m_u32TxTick = CAN_TX_TIMEOUT_MS;
            m_u8TxStart = 1U;
        } else {
            DDL_Printf("CAN is transmitting.\r\n");
        }
    }
}

/**
 * @brief  CAN receives data.
 * @param  None
 * @retval None
 */
static void CanRx(void)
{
    uint8_t i;
    uint8_t j;
    uint8_t u8RxFrameNum = 0U;
    int32_t i32Ret;

    /* Get all received frames. */
    do {
        i32Ret = CAN_GetRxFrame(CAN_UNIT, &m_astRxFrame[u8RxFrameNum]);
        if (i32Ret == LL_OK) {
            u8RxFrameNum++;
        }
    } while (i32Ret == LL_OK);

    /* Handle received frames. */
    for (i = 0U; i < u8RxFrameNum; i++) {
        DDL_Printf("CAN received %s frame with %s ID %.8x:\r\n", \
                   m_s8FrameTypeStr[m_astRxFrame[i].FDF], \
                   m_s8IDTypeStr[m_astRxFrame[i].IDE],    \
                   (unsigned int)m_astRxFrame[i].u32ID);
        for (j = 0; j < (uint8_t)m_astRxFrame[i].DLC; j++) {
            DDL_Printf(" %.2x.", m_astRxFrame[i].au8Data[j]);
            m_astRxFrame[i].au8Data[j] = 0U;
        }
        DDL_Printf("\r\n");
    }
}

/**
 * @brief  CAN interrupt callback.
 * @param  None
 * @retval None
 */
static void CAN_IrqCallback(void)
{
    stc_can_error_info_t stcErr;

    (void)CAN_GetErrorInfo(CAN_UNIT, &stcErr);
    DDL_Printf("---> CAN error type: %u, %s\r\n", stcErr.u8ErrorType, m_s8ErrorTypeStr[stcErr.u8ErrorType]);

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_BUS_OFF) == SET) {
        DDL_Printf("BUS OFF.\r\n");
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_RX_BUF_OVF) == SET) {
        DDL_Printf("RX overflow.\r\n");
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_RX_BUF_OVF);
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_TX_BUF_FULL) == SET) {
        DDL_Printf("TX buffer full.\r\n");
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_TX_ABORTED) == SET) {
        DDL_Printf("TX aborted.\r\n");
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_TX_ABORTED);
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_STB_TX) == SET) {
        DDL_Printf("STB transmitted.\r\n");
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_STB_TX);
        m_u8STBTxFlag = 1U;
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_PTB_TX) == SET) {
        DDL_Printf("PTB transmitted.\r\n");
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_PTB_TX);
        m_u8PTBTxFlag = 1U;
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_RX) == SET) {
        /* Received frame can be read here. */
        DDL_Printf("Received a frame.\r\n");
        m_u8RxFlag = 1U;
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_RX);
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_RX_BUF_WARN) == SET) {
        /* Received frames can be read here. */
        DDL_Printf("RX buffer warning.\r\n");
        m_u8RxFlag = 1U;
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_RX_BUF_WARN);
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_RX_BUF_FULL) == SET) {
        /* Received frames can be read here. */
        DDL_Printf("RX buffer full.\r\n");
        m_u8RxFlag = 1U;
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_RX_BUF_FULL);
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_RX_OVERRUN) == SET) {
        DDL_Printf("RX buffer overrun.\r\n");
        m_u8RxFlag = 1U;
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_RX_OVERRUN);
    }

    if (CAN_GetStatus(CAN_UNIT, CAN_FLAG_TEC_REC_WARN) == SET) {
        DDL_Printf("TEC or REC reached warning limit.\r\n");
        CAN_ClearStatus(CAN_UNIT, CAN_FLAG_TEC_REC_WARN);
    }
}

/**
 * @brief  SysTick interrupt handler function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    if (m_u8TxStart != 0U) {
        switch (m_u8TxProcess) {
            case CAN_TX_PROCESS_PTB:
                if (m_u8PTBTxFlag != 0U) {
                    m_u8TxStart   = 0U;
                    m_u8PTBTxFlag = 0U;
                    m_u32TxTick   = 0UL;
                }
                break;
            case CAN_TX_PROCESS_STB:
                if (m_u8STBTxFlag != 0U) {
                    m_u8TxStart   = 0U;
                    m_u8STBTxFlag = 0U;
                    m_u32TxTick   = 0UL;
                }
                break;
            case CAN_TX_PROCESS_PTB_STB:
                if ((m_u8PTBTxFlag != 0U) && (m_u8STBTxFlag != 0U)) {
                    m_u8TxStart   = 0U;
                    m_u8PTBTxFlag = 0U;
                    m_u8STBTxFlag = 0U;
                    m_u32TxTick   = 0UL;
                }
                break;
            default:
                break;
        }

        if (m_u32TxTick > 0U) {
            m_u32TxTick--;
            if (m_u32TxTick == 0UL) {
                FCG_Fcg1PeriphClockCmd(CAN_PERIPH_CLK, DISABLE);
                CanInitConfig();
                m_u8TxStart   = 0U;
                m_u8PTBTxFlag = 0U;
                m_u8STBTxFlag = 0U;
                DDL_Printf("CAN TX timeout!\r\n");
            }
        }
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
