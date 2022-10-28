/**
 *******************************************************************************
 * @file  can/can_ttcan/source/main.c
 * @brief Main program of CAN TTCAN for the Device Driver Library.
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
 * @addtogroup CAN_TTCAN
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
#define CAN_PHY_STBY_PORT               (GPIO_PORT_D)
#define CAN_PHY_STBY_PIN                (GPIO_PIN_15)

/* Acceptance filter. */
#define CAN_FILTER_SEL                  (CAN_FILTER1 | CAN_FILTER2)
#define CAN_FILTER_NUM                  (2U)

#define CAN_FILTER1_ID                  (0x701UL)
#define CAN_FILTER1_ID_MASK             (0x0UL)
#define CAN_FILTER1_ID_TYPE             (CAN_ID_STD)        /*!< Only accept frames with standard ID 0x701. */

#define CAN_FILTER2_ID                  (0x12131415UL)
#define CAN_FILTER2_ID_MASK             (0x000000F0UL)
#define CAN_FILTER2_ID_TYPE             (CAN_ID_EXT)        /*!< Accept frames with extended ID 0x121314x5. */

/* Reference message ID */
#define CAN_TTC_REF_ID                  (CAN_FILTER2_ID)
#define CAN_TTC_REF_IDE                 (1U)

/* Message ID definitions. */
#define CAN_TX_ID1                      (0x601UL)
#define CAN_TX_ID1_IDE                  (0U)

/* Size of the TX data. */
#define CAN_TX_DLC                      (CAN_DLC8)
#define CAN_TX_DATA_SIZE                (8U)

/* Max size of the RX data. */
#define CAN_RX_DATA_SIZE_MAX            (8U)

/* Timeout value */
#define CAN_TX_TIMEOUT_MS               (505U)

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

static void TtcanTx(void);
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

static __IO uint8_t m_u8RxFlag     = 0U;
static __IO uint8_t m_u8STBTxFlag  = 0U;
static __IO uint32_t m_u32TxTick   = 0UL;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of can_ttcan project
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
    stc_can_ttc_config_t stcCanTtc;
    stc_can_filter_config_t astcFilter[CAN_FILTER_NUM] = {
        {CAN_FILTER1_ID, CAN_FILTER1_ID_MASK, CAN_FILTER1_ID_TYPE},
        {CAN_FILTER2_ID, CAN_FILTER2_ID_MASK, CAN_FILTER2_ID_TYPE},
    };

    /* Initializes CAN. */
    (void)CAN_StructInit(&stcCanInit);
    stcCanInit.stcBitCfg.u32Prescaler = 1U;
    stcCanInit.stcBitCfg.u32TimeSeg1  = 12U;
    stcCanInit.stcBitCfg.u32TimeSeg2  = 4U;
    stcCanInit.stcBitCfg.u32SJW       = 4U;
    stcCanInit.pstcFilter             = astcFilter;
    stcCanInit.u16FilterSelect        = CAN_FILTER_SEL;
    stcCanInit.u8WorkMode             = CAN_WORK_MD_NORMAL;

    (void)CAN_TTC_StructInit(&stcCanTtc);
    stcCanTtc.u32RefMsgID         = CAN_TTC_REF_ID;
    stcCanTtc.u32RefMsgIDE        = CAN_TTC_REF_IDE;
    stcCanTtc.u8NTUPrescaler      = CAN_TTC_NTU_PRESCALER8;
    stcCanTtc.u16TriggerType      = CAN_TTC_TRIG_SINGLESHOT_TX_TRIG;
    stcCanTtc.u16TxTriggerTime    = 31250U;
    stcCanTtc.u16WatchTriggerTime = 65000U;
    stcCanInit.pstcCanTtc = &stcCanTtc;

    FCG_Fcg1PeriphClockCmd(CAN_PERIPH_CLK, ENABLE);
    (void)CAN_Init(CAN_UNIT, &stcCanInit);
    /* Enable the interrupts, the status flags can be read. */
    CAN_IntCmd(CAN_UNIT, CAN_INT_ALL, DISABLE);
    CAN_IntCmd(CAN_UNIT, CAN_INT_SEL, ENABLE);
    CAN_TTC_IntCmd(CAN_UNIT, CAN_TTC_INT_ALL, ENABLE);
    /* Enable TTCAN, local counter of TTCAN starts counting. */
    CAN_TTC_Cmd(CAN_UNIT, ENABLE);
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
static void TtcanTx(void)
{
    uint8_t i;
    stc_can_tx_frame_t stcTx;

    for (i = 0U; i < CAN_TX_DATA_SIZE; i++) {
        stcTx.au8Data[i] = i + 0x10U;
    }

    /* Frame with CAN_TX_ID1 */
    stcTx.u32Ctrl = 0x0UL;
    stcTx.u32ID   = CAN_TX_ID1;
    stcTx.IDE     = CAN_TX_ID1_IDE;
    stcTx.DLC     = CAN_TX_DLC;

    CAN_TTC_SetTriggerType(CAN_UNIT, CAN_TTC_TRIG_SINGLESHOT_TX_TRIG);
    (void)CAN_TTC_FillTxFrame(CAN_UNIT, CAN_TTC_TX_BUF_STB1, &stcTx);
    m_u32TxTick = CAN_TX_TIMEOUT_MS;
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
        if (m_astRxFrame[i].u32ID == CAN_TTC_REF_ID) {
            DDL_Printf("CAN received reference message with %s ID %.8x:\r\n", \
                       m_s8IDTypeStr[m_astRxFrame[i].IDE],    \
                       (unsigned int)m_astRxFrame[i].u32ID);
            TtcanTx();
        } else {
            DDL_Printf("CAN received %s frame with %s ID %.8x, cycle time %u:\r\n", \
                       m_s8FrameTypeStr[m_astRxFrame[i].FDF], \
                       m_s8IDTypeStr[m_astRxFrame[i].IDE],    \
                       (unsigned int)m_astRxFrame[i].u32ID,   \
                       (uint16_t)m_astRxFrame[i].CYCLE_TIME);
        }
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

    if (CAN_TTC_GetStatus(CAN_UNIT, CAN_TTC_FLAG_TIME_TRIG) == SET) {
        DDL_Printf("Time trigger interrupt.\r\n");
        CAN_TTC_ClearStatus(CAN_UNIT, CAN_TTC_FLAG_TIME_TRIG);
    }

    if (CAN_TTC_GetStatus(CAN_UNIT, CAN_TTC_FLAG_TRIG_ERR) == SET) {
        DDL_Printf("Trigger error interrupt.\r\n");
    }

    if (CAN_TTC_GetStatus(CAN_UNIT, CAN_TTC_FLAG_WATCH_TRIG) == SET) {
        DDL_Printf("Watch trigger interrupt.\r\n");
        CAN_TTC_ClearStatus(CAN_UNIT, CAN_TTC_FLAG_WATCH_TRIG);
    }
}

/**
 * @brief  SysTick interrupt handler function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    if (m_u8STBTxFlag != 0U) {
        m_u32TxTick    = 0UL;
        m_u8STBTxFlag  = 0U;
    }

    if (m_u32TxTick > 0U) {
        m_u32TxTick--;
        if (m_u32TxTick == 0U) {
            FCG_Fcg1PeriphClockCmd(CAN_PERIPH_CLK, DISABLE);
            CanInitConfig();
            m_u8STBTxFlag  = 0U;
            DDL_Printf("STB TX timeout.\r\n");
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
