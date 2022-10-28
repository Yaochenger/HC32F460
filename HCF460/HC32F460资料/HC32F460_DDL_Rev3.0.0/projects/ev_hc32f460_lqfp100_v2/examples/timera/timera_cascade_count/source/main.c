/**
 *******************************************************************************
 * @file  timera/timera_cascade_count/source/main.c
 * @brief Main program TimerA cascade count for the Device Driver Library.
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
 * @addtogroup TIMERA_Cascade_Count
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*
 * TimerA unit and channel definitions for this example.
 * Symmetric units: unit 1 and 2; unit 3 and 4; unit 5 and 6.
 */
#define TMRA_LOW_UNIT                   (CM_TMRA_1)
#define TMRA_HIGH_UNIT                  (CM_TMRA_2)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1 | FCG2_PERIPH_TMRA_2)

/*
 * Specifies the count mode of the low unit that you need. @ref TMRA_Count_Mode
 */
#define TMRA_LOW_UNIT_MD                (TMRA_MD_TRIANGLE)

/*
 * Specifies the count direction of the low unit that you need. @ref TMRA_Count_Dir
 */
#define TMRA_LOW_UNIT_DIR               (TMRA_DIR_DOWN)

/* The clock for high unit. */
#if (TMRA_LOW_UNIT_MD == TMRA_MD_SAWTOOTH)
#if (TMRA_LOW_UNIT_DIR == TMRA_DIR_UP)
#define TMRA_HIGH_UNIT_CNT_UP_COND      (TMRA_CNT_UP_COND_SYM_OVF)
#elif (TMRA_LOW_UNIT_DIR == TMRA_DIR_DOWN)
#define TMRA_HIGH_UNIT_CNT_UP_COND      (TMRA_CNT_UP_COND_SYM_UDF)
#endif
#elif (TMRA_LOW_UNIT_MD == TMRA_MD_TRIANGLE)
/* Both TMRA_CNT_UP_COND_SYM_OVF and TMRA_CNT_UP_COND_SYM_UDF can be used. */
#define TMRA_HIGH_UNIT_CNT_UP_COND      (TMRA_CNT_UP_COND_SYM_UDF)
#endif

/* The clock divider of low unit . @ref TMRA_Clock_Divider */
#define TMRA_LOW_UNIT_CLK_DIV           (TMRA_CLK_DIV8)

/*
 * Specify the timing period value of low unit. \
 *   Set the compare value of high unit according to the timing period value of low unit.
 * In this example:
 *   TimerA low unit timing period: 1 millisecond.
 *   The clock of low unit: PCLK0(8MHz) / 8 = 1MHz
 *   Number of counting cycles of low unit: (1000UL)
 */
#if (TMRA_LOW_UNIT_MD == TMRA_MD_SAWTOOTH)
#define TMRA_LOW_UNIT_PERIOD_VAL        (1000U - 1U)
#elif (TMRA_LOW_UNIT_MD == TMRA_MD_TRIANGLE)
#define TMRA_LOW_UNIT_PERIOD_VAL        (1000U / 2U)
#endif

/* Timing period is 10 milliseconds. */
#if (TMRA_HIGH_UNIT_MD == TMRA_MD_SAWTOOTH)
#define TMRA_HIGH_UNIT_PERIOD_VAL       (10U - 1U)
#elif (TMRA_HIGH_UNIT_MD == TMRA_MD_TRIANGLE)
#define TMRA_HIGH_UNIT_PERIOD_VAL       (10U / 2U)
#endif

/* Definitions of interrupt of TimerA high unit. */
#define TMRA_HIGH_UNIT_INT_TYPE         (TMRA_INT_OVF)
#define TMRA_HIGH_UNIT_INT_SRC          (INT_SRC_TMRA_2_OVF)
#define TMRA_HIGH_UNIT_INT_IRQn         (INT080_IRQn)
#define TMRA_HIGH_UNIT_INT_PRIO         (DDL_IRQ_PRIO_03)
#define TMRA_HIGH_UNIT_INT_FLAG         (TMRA_FLAG_OVF)

/*
 * Definitions about TimerA interrupt for the example.
 */
#define TMRA_INT_UNIT                   (TMRA_HIGH_UNIT)
#define TMRA_INT_TYPE                   (TMRA_HIGH_UNIT_INT_TYPE)
#define TMRA_INT_SRC                    (TMRA_HIGH_UNIT_INT_SRC)
#define TMRA_INT_IRQn                   (TMRA_HIGH_UNIT_INT_IRQn)
#define TMRA_INT_PRIO                   (TMRA_HIGH_UNIT_INT_PRIO)
#define TMRA_INT_FLAG                   (TMRA_HIGH_UNIT_INT_FLAG)

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

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timera_cascade_count project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Configures indicator. */
    IndicateConfig();
    /* Configures TimerA. */
    TmrAConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Starts TimerA. */
    TMRA_Start(TMRA_HIGH_UNIT);
    TMRA_Start(TMRA_LOW_UNIT);

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

    /* 2. Initialize TimerA low unit . */
    (void)TMRA_StructInit(&stcTmraInit);
    stcTmraInit.sw_count.u16ClockDiv  = TMRA_LOW_UNIT_CLK_DIV;
    stcTmraInit.sw_count.u16CountMode = TMRA_LOW_UNIT_MD;
    stcTmraInit.sw_count.u16CountDir  = TMRA_LOW_UNIT_DIR;
    stcTmraInit.u32PeriodValue = TMRA_LOW_UNIT_PERIOD_VAL;
    (void)TMRA_Init(TMRA_LOW_UNIT, &stcTmraInit);

    /* 3. Initialize TimerA high unit . */
    (void)TMRA_StructInit(&stcTmraInit);
    stcTmraInit.u8CountSrc = TMRA_CNT_SRC_HW;
    stcTmraInit.hw_count.u16CountUpCond = TMRA_HIGH_UNIT_CNT_UP_COND;
    stcTmraInit.u32PeriodValue = TMRA_HIGH_UNIT_PERIOD_VAL;
    (void)TMRA_Init(TMRA_HIGH_UNIT, &stcTmraInit);

    /* 4. Configures IRQ if needed. */
    TmrAIrqConfig();
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
    TMRA_IntCmd(TMRA_INT_UNIT, TMRA_INT_TYPE, ENABLE);
}

/**
 * @brief  TimerA interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMRA_IrqCallback(void)
{
    TMRA_ClearStatus(TMRA_INT_UNIT, TMRA_INT_FLAG);
    INDICATE_OUT_TOGGLE();
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
