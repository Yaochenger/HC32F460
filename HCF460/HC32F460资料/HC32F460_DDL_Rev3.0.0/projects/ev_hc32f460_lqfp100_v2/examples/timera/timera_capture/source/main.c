/**
 *******************************************************************************
 * @file  timera/timera_capture/source/main.c
 * @brief Main program TimerA capture for the Device Driver Library.
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
 * @addtogroup TIMERA_Capture
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* TimerA unit and channel definitions for this example. */
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_CH                         (TMRA_CH4)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)
#define TMRA_AOS_CAPT_REG               (AOS_TMRA_1)

/*
 * Specify conditions for TimerA capturing in this example.
 * The conditions are included in @ref TMRA_Capture_Cond
 */
#define TMRA_CAPT_COND                  (TMRA_CAPT_COND_PWM_RISING)

/* Feasibility check. */
#if (TMRA_CAPT_COND & (TMRA_CAPT_COND_TRIG_RISING | TMRA_CAPT_COND_TRIG_FALLING))
#if (TMRA_CH != TMRA_CH4)
#error "Only channel 4 of TimerA supports capturing the edge of TRIG pin."
#endif
#endif

/* Specifies the pin or the event to be used according to the capture-condition. */
#if (TMRA_CAPT_COND & (TMRA_CAPT_COND_PWM_RISING | TMRA_CAPT_COND_PWM_FALLING))
#define TMRA_CAPT_PWM_PORT              (GPIO_PORT_A)
#define TMRA_CAPT_PWM_PIN               (GPIO_PIN_11)
#define TMRA_CAPT_PWM_PIN_FUNC          (GPIO_FUNC_4)
#endif

#if (TMRA_CAPT_COND & TMRA_CAPT_COND_EVT)
/* BSK key K1 can generate the event. */
#define TMRA_CAPT_EVT                   (EVT_SRC_PORT_EIRQ12)
#endif

#if (TMRA_CAPT_COND & (TMRA_CAPT_COND_TRIG_RISING | TMRA_CAPT_COND_TRIG_FALLING))
#define TMRA_CAPT_TRIG_PORT             (GPIO_PORT_A)
#define TMRA_CAPT_TRIG_PIN              (GPIO_PIN_12)
#define TMRA_CAPT_TRIG_PIN_FUNC         (GPIO_FUNC_4)
#endif

/* Specifies clock divider that you need. @ref TMRA_Clock_Divider */
#define TMRA_CLK_DIV                    (TMRA_CLK_DIV4)

/* Definitions of interrupt. */
#define TMRA_INT_IRQn                   (INT080_IRQn)
#define TMRA_INT_SRC                    (INT_SRC_TMRA_1_CMP)
#define TMRA_INT_PRIO                   (DDL_IRQ_PRIO_03)
#define TMRA_INT_TYPE                   (TMRA_INT_CMP_CH4)
#define TMRA_INT_FLAG                   (TMRA_FLAG_CMP_CH4)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void TmrAConfig(void);
static void TmrACaptureCondConfig(void);
static void TmrAIrqConfig(void);
static void TMRA_IrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timera_capture project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Initializes UART for debug printing. Baudrate is 19200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 19200U, BSP_PRINTF_Preinit);
    /* Configures TimerA. */
    TmrAConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    /* Starts TimerA. */
    TMRA_Start(TMRA_UNIT);

    /***************** Configuration end, application start **************/

    for (;;) {
        /* See TMRA_IrqCallback */
    }
}

/**
 * @brief  TimerA configuration.
 * @param  None
 * @retval None
 */
static void TmrAConfig(void)
{
    stc_tmra_init_t stcTmraInit;

    /* 1. Enable TimerA peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMRA_PERIPH_CLK, ENABLE);

    /* 2. Set a default initialization value for stcTmraInit. */
    (void)TMRA_StructInit(&stcTmraInit);

    /* 3. Modifies the initialization values depends on the application. */
    stcTmraInit.sw_count.u16ClockDiv = TMRA_CLK_DIV;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);

    /* 4. Set function mode as capturing mode. */
    TMRA_SetFunc(TMRA_UNIT, TMRA_CH, TMRA_FUNC_CAPT);

    /* 5. Configures the capture condition. */
    TmrACaptureCondConfig();

    /* 6. Configures IRQ if needed. */
    TmrAIrqConfig();
}

/**
 * @brief  Capture condition configuration.
 * @param  None
 * @retval None
 */
static void TmrACaptureCondConfig(void)
{
#if (TMRA_CAPT_COND & (TMRA_CAPT_COND_PWM_RISING | TMRA_CAPT_COND_PWM_FALLING))
    GPIO_SetFunc(TMRA_CAPT_PWM_PORT, TMRA_CAPT_PWM_PIN, TMRA_CAPT_PWM_PIN_FUNC);
#endif

#if (TMRA_CAPT_COND & TMRA_CAPT_COND_EVT)
    BSP_KEY_Init();
    /* Enable AOS function. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    /* Set the event for TimerA capturing. */
    AOS_SetTriggerEventSrc(TMRA_AOS_CAPT_REG, TMRA_CAPT_EVT);
#endif

#if (TMRA_CAPT_COND & (TMRA_CAPT_COND_TRIG_RISING | TMRA_CAPT_COND_TRIG_FALLING))
    GPIO_SetFunc(TMRA_CAPT_TRIG_PORT, TMRA_CAPT_TRIG_PIN, TMRA_CAPT_TRIG_PIN_FUNC);
    /* The capture-condition is configured via channel 3 when the capture-condition is the edge of pin TRIG. */
    TMRA_HWCaptureCondCmd(TMRA_UNIT, TMRA_CH3, TMRA_CAPT_COND, ENABLE);
#else
    TMRA_HWCaptureCondCmd(TMRA_UNIT, TMRA_CH, TMRA_CAPT_COND, ENABLE);
#endif
}

/**
 * @brief  TimerA interrupt configuration.
 * @param  None
 * @retval None
 */
static void TmrAIrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = TMRA_INT_SRC;
    stcIrq.enIRQn      = TMRA_INT_IRQn;
    stcIrq.pfnCallback = &TMRA_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);

    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, TMRA_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    /* Enable the specified interrupts of TimerA. */
    TMRA_IntCmd(TMRA_UNIT, TMRA_INT_TYPE, ENABLE);
}

/**
 * @brief  TimerA counter compare match interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMRA_IrqCallback(void)
{
    /* A capture occurred */
    /* Get capture value by calling function TMRA_GetCompareValue. */
    TMRA_ClearStatus(TMRA_UNIT, TMRA_INT_FLAG);
    DDL_Printf("A capture occurred. Captured value is 0x%.4x\r\n",
               (unsigned int)TMRA_GetCompareValue(TMRA_UNIT, TMRA_CH));
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
