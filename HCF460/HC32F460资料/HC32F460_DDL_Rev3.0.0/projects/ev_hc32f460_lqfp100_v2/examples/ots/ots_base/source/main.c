/**
 *******************************************************************************
 * @file  ots/ots_base/source/main.c
 * @brief Main program OTS base for the Device Driver Library.
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
 * @addtogroup OTS_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*
 * Function of this example.
 * This example is going to get the temperature of the chip inside.
 */

/*
 * Specifies a clock source for OTS in this example.
 * 'OTS_CLK_SEL' can be defined as 'OTS_CLK_XTAL' or 'OTS_CLK_HRC'. */
#define OTS_CLK_SEL                 (OTS_CLK_XTAL)

/*
 * Function control of OTS.
 * Defines the following macro as non-zero to enable the corresponding function.
 *
 * 'OTS_USE_INTERRUPT': Interrupt function control.
 * 'OTS_USE_TRIG': Hardware trigger conditions control. The condition that used to start OTS.
 */
#define OTS_USE_TRIG                    (1U)

#if (OTS_USE_TRIG > 0U)
#define OTS_USE_INTERRUPT               (OTS_USE_TRIG)
#else
#define OTS_USE_INTERRUPT               (1U)
#endif

/*
 * Definitions about OTS interrupt for the example.
 * OTS independent IRQn: [INT000_IRQn, INT031_IRQn], [INT116_IRQn, INT121_IRQn].
 */
#if (OTS_USE_INTERRUPT > 0U)
#define OTS_INT_PRIO                    (DDL_IRQ_PRIO_03)
#define OTS_INT_SRC                     (INT_SRC_OTS)
#define OTS_INT_IRQn                    (INT110_IRQn)
#endif /* #if (OTS_USE_INTERRUPT > 0U) */

/* OTS parameters, slope K and offset M. Different chip, different parameters. */
#define OTS_XTAL_K                      (737272.73F)
#define OTS_XTAL_M                      (27.55F)
#define OTS_HRC_K                       (3002.59F)
#define OTS_HRC_M                       (27.92F)

/* Timeout value. */
#define OTS_TIMEOUT_VAL                 (10000U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void OtsConfig(void);
static void OtsInitConfig(void);
static void OtsClockConfig(void);

#if (OTS_USE_INTERRUPT > 0U)
static void OtsIrqConfig(void);
static void OTS_IrqCallback(void);
#endif

#if (OTS_USE_TRIG > 0U)
static void OtsTriggerConfig(void);
#endif

#if ((OTS_USE_INTERRUPT > 0U) || (OTS_USE_TRIG > 0U))
static void OtsStart(void);
#endif

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
#if (OTS_USE_INTERRUPT > 0U)
__IO static uint8_t m_u8OtsIntFlag = 0U;
__IO static float32_t m_f32Temperature;
#else
static float32_t m_f32Temperature;
#endif

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of ots_base project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* The system clock is MRC(8MHz) by default. */

    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Initializes UART for debug printing. Baudrate is 19200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 19200U, BSP_PRINTF_Preinit);
    /* Configures OTS. */
    OtsConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

#if ((OTS_USE_INTERRUPT > 0U) || (OTS_USE_TRIG > 0U))
    /* Starts OTS. */
    OtsStart();
#endif

    /***************** Configuration end, application start **************/

    for (;;) {
#if (OTS_USE_INTERRUPT > 0U)
        if (m_u8OtsIntFlag != 0U) {
            DDL_Printf("Temperature: %u\r\n", (uint16_t)m_f32Temperature);
#if (OTS_USE_TRIG == 0U)
            DDL_DelayMS(1000U);
            OTS_Start();
#endif
            m_u8OtsIntFlag = 0U;
        }
#else
        (void)OTS_Polling(&m_f32Temperature, OTS_TIMEOUT_VAL);
        DDL_Printf("Temperature: %u\r\n", (uint16_t)m_f32Temperature);
        m_f32Temperature = 0.0F;
        DDL_DelayMS(1000U);
#endif
    }
}

/**
 * @brief  OTS configuration.
 * @param  None
 * @retval None
 */
static void OtsConfig(void)
{
    OtsInitConfig();
    OtsClockConfig();
}

/**
 * @brief  OTS initialization configuration.
 * @param  None
 * @retval None
 */
static void OtsInitConfig(void)
{
    stc_ots_init_t stcOTSInit;

    (void)OTS_StructInit(&stcOTSInit);
    stcOTSInit.u16ClockSrc = OTS_CLK_SEL;

#if (OTS_CLK_SEL == OTS_CLK_XTAL)
    stcOTSInit.f32SlopeK   = OTS_XTAL_K;
    stcOTSInit.f32OffsetM  = OTS_XTAL_M;
#else
    stcOTSInit.f32SlopeK   = OTS_HRC_K;
    stcOTSInit.f32OffsetM  = OTS_HRC_M;
#endif /* #if (OTS_CLK_SEL == OTS_CLK_XTAL) */

    /* 1. Enable OTS peripheral clock. */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_OTS, ENABLE);

    /* 2. Initialize OTS. */
    (void)OTS_Init(&stcOTSInit);

#if (OTS_USE_INTERRUPT > 0U)
    OtsIrqConfig();
#endif

#if (OTS_USE_TRIG > 0U)
    OtsTriggerConfig();
#endif
}

/**
 * @brief  OTS clock configuration.
 * @param  None
 * @retval None
 */
static void OtsClockConfig(void)
{
#if (OTS_CLK_SEL == OTS_CLK_HRC)
    /* 1. Enable HRC for OTS. */
    (void)CLK_HrcCmd(ENABLE);
    /* 2. Enable XTAL32 when clock source of OTS is HRC. */
    (void)CLK_Xtal32Cmd(ENABLE);
#else
    /* Enable XTAL for OTS. */
    (void)CLK_XtalCmd(ENABLE);
#endif

    /* Enable LRC for OTS. */
    (void)CLK_LrcCmd(ENABLE);
}

#if (OTS_USE_INTERRUPT > 0U)
/**
 * @brief  OTS interrupt configuration.
 * @param  None
 * @retval None
 */
static void OtsIrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = OTS_INT_SRC;
    stcIrq.enIRQn      = OTS_INT_IRQn;
    stcIrq.pfnCallback = &OTS_IrqCallback;

    /* Independent interrupt. */
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, OTS_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    /* Enable the specified interrupts of OTS. */
    OTS_IntCmd(ENABLE);
}

/**
 * @brief  OTS interrupt callback function.
 * @param  None
 * @retval None
 */
void OTS_IrqCallback(void)
{
    m_f32Temperature = OTS_CalculateTemp();
    m_u8OtsIntFlag   = 1U;
}
#endif /* #if (OTS_USE_INTERRUPT > 0U) */

#if (OTS_USE_TRIG > 0U)
/**
 * @brief  Specifies event 'EVT_SRC_TMR0_1_CMP_A' of TIMER0 unit 1 channel A as the trigger source event of OTS. \
 *         Event 'EVT_SRC_TMR0_1_CMP_A' occurs every second.
 * @param  None
 * @retval None
 */
static void OtsTriggerConfig(void)
{
    /*
     * If a peripheral is used to generate the event which is used as a start trigger condition of OTS, \
     *   call the API of the peripheral to configure the peripheral.
     * The following operations are only used in this example.
     */

    stc_tmr0_init_t stcTMR0Init;

    /* Initials TIMER0. */
    (void)TMR0_StructInit(&stcTMR0Init);
    stcTMR0Init.u32ClockSrc     = TMR0_CLK_SRC_INTERN_CLK;
    stcTMR0Init.u32ClockDiv     = TMR0_CLK_DIV256;
    stcTMR0Init.u32Func         = TMR0_FUNC_CMP;
    stcTMR0Init.u16CompareValue = 31250UL;

    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR0_1, ENABLE);
    (void)TMR0_Init(CM_TMR0_1, TMR0_CH_A, &stcTMR0Init);

    /* Specifies event 'EVT_SRC_TMR0_1_CMP_A' as the trigger source event of OTS. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    AOS_SetTriggerEventSrc(AOS_OTS, EVT_SRC_TMR0_1_CMP_A);
}
#endif /* #if (OTS_USE_TRIG > 0U) */

#if ((OTS_USE_INTERRUPT > 0U) || (OTS_USE_TRIG > 0U))
/**
 * @brief  Start OTS.
 * @param  None
 * @retval None
 */
static void OtsStart(void)
{
    /*
     * If a peripheral is used to generate the event which is used as a start trigger condition of OTS, \
     *   call the API of the peripheral to start the peripheral here or anywhere else you need.
     * The following operations are only used in this example.
     */

#if (OTS_USE_TRIG > 0U)
    TMR0_Start(CM_TMR0_1, TMR0_CH_A);
#elif (OTS_USE_INTERRUPT > 0U)
    OTS_Start();
#endif
}
#endif

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
