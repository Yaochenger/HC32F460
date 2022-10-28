/**
 *******************************************************************************
 * @file  timera/timera_base_timer/source/main.c
 * @brief Main program TimerA base timer for the Device Driver Library.
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
 * @addtogroup TIMERA_Base_Timer
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* TimerA unit definitions for this example. */
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)

/*
 * Use hardware trigger if needed. None-zero to enable.
 * Hardware trigger conditions control. The conditions that can start TimerA, \
 * stop TimerA or clear counter of TimerA.
 */
#define TMRA_USE_HW_TRIG                (0U)

/* The divider of the clock source. @ref TMRA_Clock_Divider */
#define TMRA_CLK_DIV                    (TMRA_CLK_DIV8)

/* The counting mode of TimerA. @ref TMRA_Count_Mode */
#define TMRA_MD                         (TMRA_MD_TRIANGLE)

#if (TMRA_MD == TMRA_MD_SAWTOOTH)
/* The counting direction of TimerA. @ref TMRA_Count_Dir */
#define TMRA_DIR                        (TMRA_DIR_DOWN)
#endif

/*
 * In this example:
 *   System clock is MRC@8MHz.
 *   TimerA clock is PCLK0@8MHz.
 *   Timing period is 1ms.
 * A simple formula for calculating the compare value is:
 *   Sawtooth mode:
 *     TmrAPeriodVal = (TmrAPeriod(us) * [TmrAClockSource(MHz) / TmrAClockDivider]) - 1.
 *   Triangle mode:
 *     TmrAPeriodVal = (TmrAPeriod(us) * [TmrAClockSource(MHz) / TmrAClockDivider]) / 2.
 */
#if (TMRA_MD == TMRA_MD_SAWTOOTH)
#define TMRA_PERIOD_VAL                 (1000U - 1U)
#elif (TMRA_MD == TMRA_MD_TRIANGLE)
#define TMRA_PERIOD_VAL                 (1000U / 2U)
#endif

/*
 * Definitions about TimerA interrupt for the example.
 */
#define TMRA_INT_PRIO                   (DDL_IRQ_PRIO_03)
#define TMRA_INT_IRQn                   (INT080_IRQn)
#if (TMRA_MD == TMRA_MD_SAWTOOTH)
#if (TMRA_DIR == TMRA_DIR_UP)
#define TMRA_INT_TYPE                   (TMRA_INT_OVF)
#define TMRA_INT_SRC                    (INT_SRC_TMRA_1_OVF)
#define TMRA_INT_FLAG                   (TMRA_FLAG_OVF)
#elif (TMRA_DIR == TMRA_DIR_DOWN)
#define TMRA_INT_TYPE                   (TMRA_INT_UDF)
#define TMRA_INT_SRC                    (INT_SRC_TMRA_1_UDF)
#define TMRA_INT_FLAG                   (TMRA_FLAG_UDF)
#endif
#elif (TMRA_MD == TMRA_MD_TRIANGLE)
/* Both TMRA_INT_OVF and TMRA_INT_UDF can be used. */
#define TMRA_INT_TYPE                   (TMRA_INT_OVF)
#define TMRA_INT_SRC                    (INT_SRC_TMRA_1_OVF)
#define TMRA_INT_FLAG                   (TMRA_FLAG_OVF)
#endif

/*
 * Specify the hardware trigger conditions if enabled(TMRA_USE_HW_TRIG > 0U).
 * 'TMRA_START_COND' specifies the condition of starting TimerA.
 * 'TMRA_STOP_COND' specifies the condition of stopping TimerA.
 * NOTE:
 *   CANNOT specify a condition as both start condition and stop condition.
 */
#if (TMRA_USE_HW_TRIG > 0U)
#define TMRA_START_COND                 (TMRA_START_COND_EVT)
/* BSP key K1 can generate the event 'EVT_SRC_PORT_EIRQ12' */
#define TMRA_TRIG_EVT                   (EVT_SRC_PORT_EIRQ12)

#define TMRA_STOP_COND                  (TMRA_STOP_COND_TRIG_FALLING)
#define TMRA_TRIG_PORT                  (GPIO_PORT_E)
#define TMRA_TRIG_PIN                   (GPIO_PIN_07)
#define TMRA_TRIG_PIN_FUNC              (GPIO_FUNC_4)
#endif /* #if (TMRA_USE_HW_TRIG > 0U) */

/* Indicate pin definition in this example. */
#define INDICATE_PORT                   (GPIO_PORT_A)
#define INDICATE_PIN                    (GPIO_PIN_02)
#define INDICATE_OUT_TOGGLE()           (GPIO_TogglePins(INDICATE_PORT, INDICATE_PIN))

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void IndicateConfig(void);

static void TmrAConfig(void);
static void TmrAIrqConfig(void);
static void TMRA_IrqCallback(void);

#if (TMRA_USE_HW_TRIG > 0U)
static void TmrATriggerCondConfig(void);
#endif

static void TmrAStart(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timera_base_timer project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
#if (TMRA_USE_HW_TRIG > 0U)
    /* BSP key K1 can trigger TimerA start. */
    BSP_KEY_Init();
#endif /* #if (TMRA_USE_HW_TRIG > 0U) */
    /* Configures indicator. */
    IndicateConfig();
    /* Configures TimerA. */
    TmrAConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    /* Starts TimerA. */
    TmrAStart();

    /***************** Configuration end, application start **************/

    for (;;) {
        /* See TMRA_IrqCallback in this file. */
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
    stcTmraInit.sw_count.u16ClockDiv  = TMRA_CLK_DIV;
    stcTmraInit.sw_count.u16CountMode = TMRA_MD;
#if (TMRA_MD == TMRA_MD_SAWTOOTH)
    stcTmraInit.sw_count.u16CountDir  = TMRA_DIR;
#endif
    stcTmraInit.u32PeriodValue = TMRA_PERIOD_VAL;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);

    /* 4. Configures IRQ if needed. */
    TmrAIrqConfig();

#if (TMRA_USE_HW_TRIG > 0U)
    /* 5. Configures hardware trigger condition if needed. */
    TmrATriggerCondConfig();
#endif /* #if (TMRA_USE_HW_TRIG > 0U) */
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
    TMRA_ClearStatus(TMRA_UNIT, TMRA_INT_FLAG);
    INDICATE_OUT_TOGGLE();
}

#if (TMRA_USE_HW_TRIG > 0U)
/**
 * @brief  Trigger condition configuration.
 * @param  None
 * @retval None
 */
static void TmrATriggerCondConfig(void)
{
    /* TimerA start condition. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    AOS_SetTriggerEventSrc(AOS_TMRA_0, TMRA_TRIG_EVT);
    TMRA_HWStartCondCmd(TMRA_UNIT, TMRA_START_COND, ENABLE);

    /* TimerA stop condition. */
    GPIO_SetFunc(TMRA_TRIG_PORT, TMRA_TRIG_PIN, TMRA_TRIG_PIN_FUNC);
    /* Configures the filter of pin TRIG if needed. */
    TMRA_SetFilterClockDiv(TMRA_UNIT, TMRA_PIN_TRIG, TMRA_FILTER_CLK_DIV64);
    TMRA_FilterCmd(TMRA_UNIT, TMRA_PIN_TRIG, ENABLE);
    TMRA_HWStopCondCmd(TMRA_UNIT, TMRA_STOP_COND, ENABLE);
}
#endif

/**
 * @brief  Start TimerA.
 * @param  None
 * @retval None
 */
static void TmrAStart(void)
{
    /*
     * If a peripheral is used to generate the event which is used as a hardware trigger condition of TimerA, \
     *   call the API of the peripheral to start the peripheral here or anywhere else you need.
     * The following operations are only used in this example.
     */

#if ((TMRA_USE_HW_TRIG == 0U) || \
     ((TMRA_USE_HW_TRIG > 0U) && (TMRA_START_COND == TMRA_START_COND_INVD)))
    TMRA_Start(TMRA_UNIT);
#else
    /* Press BSP key K1 to start TimerA. */
#endif
}

/**
 * @brief  Indicator configuration.
 * @param  None
 * @retval None
 */
static void IndicateConfig(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    (void)GPIO_Init(INDICATE_PORT, INDICATE_PIN, &stcGpioInit);
    /* Output enable */
    GPIO_OutputCmd(INDICATE_PORT, INDICATE_PIN, ENABLE);
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
