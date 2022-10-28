/**
 *******************************************************************************
 * @file  timera/timera_pwm/source/main.c
 * @brief Main program TimerA PWM for the Device Driver Library.
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
 * @addtogroup TIMERA_PWM
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Function of this example. */
#define APP_FUNC_NORMAL_SINGLE_PWM          (0U)
#define APP_FUNC_SINGLE_EDGE_ALIGNED_PWM    (1U)
#define APP_FUNC_TOW_EDGE_SYMMETRIC_PWM     (2U)

/* Specify the function of the example. */
#define APP_FUNC                            (APP_FUNC_NORMAL_SINGLE_PWM)

/*
 * Define the configurations of PWM according to the function that selected.
 * In this example:
 *   1. System clock is XTAL(8MHz).
 *   2. Clock source of TimerA is PCLK(8MHz by default) and divided by 1.
 *   3. About PWM:
 *      APP_FUNC_NORMAL_SINGLE_PWM: frequency 200KHz, high duty 30%
 *      APP_FUNC_SINGLE_EDGE_ALIGNED_PWM: frequency 100KH, high duty 30%, 55%
 *      APP_FUNC_TOW_EDGE_SYMMETRIC_PWM: frequency 50KHz, high duty 50%, 40%
 *
 * Sawtooth mode:
 *   Calculate the period value according to the frequency:
 *     PeriodVal = (TimerAClockFrequency(Hz) / PWMFreq) - 1
 *   Calculate the compare value according to the duty ratio:
 *     CmpVal = ((PeriodVal + 1) * Duty) - 1
 *
 * Triangle mode:
 *   Calculate the period value according to the frequency:
 *     PeriodVal = (TimerAClockFrequency(Hz) / (PWMFreq * 2))
 *   Calculate the compare value according to the duty ratio:
 *     CmpVal = (PeriodVal * Duty)
 */
#if (APP_FUNC == APP_FUNC_NORMAL_SINGLE_PWM)
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)
#define TMRA_PWM_CH                     (TMRA_CH2)

#define TMRA_PWM_PORT                   (GPIO_PORT_A)
#define TMRA_PWM_PIN                    (GPIO_PIN_09)
#define TMRA_PWM_PIN_FUNC               (GPIO_FUNC_4)

#define TMRA_MD                         (TMRA_MD_SAWTOOTH)
#define TMRA_DIR                        (TMRA_DIR_UP)
#define TMRA_PERIOD_VAL                 (40U - 1U)
#define TMRA_PWM_CMP_VAL                (12U - 1U)

#elif (APP_FUNC == APP_FUNC_SINGLE_EDGE_ALIGNED_PWM)
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)
#define TMRA_PWMX_CH                    (TMRA_CH1)
#define TMRA_PWMY_CH                    (TMRA_CH2)

#define TMRA_PWMX_PORT                  (GPIO_PORT_A)
#define TMRA_PWMX_PIN                   (GPIO_PIN_08)
#define TMRA_PWMX_PIN_FUNC              (GPIO_FUNC_4)
#define TMRA_PWMY_PORT                  (GPIO_PORT_A)
#define TMRA_PWMY_PIN                   (GPIO_PIN_09)
#define TMRA_PWMY_PIN_FUNC              (GPIO_FUNC_4)

#define TMRA_MD                         (TMRA_MD_SAWTOOTH)
#define TMRA_DIR                        (TMRA_DIR_UP)
#define TMRA_PERIOD_VAL                 (80U - 1U)
#define TMRA_PWMX_CMP_VAL               (24U - 1U)
#define TMRA_PWMY_CMP_VAL               (44U - 1U)

#elif (APP_FUNC == APP_FUNC_TOW_EDGE_SYMMETRIC_PWM)
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)
#define TMRA_PWMX_CH                    (TMRA_CH1)
#define TMRA_PWMY_CH                    (TMRA_CH2)

#define TMRA_PWMX_PORT                  (GPIO_PORT_A)
#define TMRA_PWMX_PIN                   (GPIO_PIN_08)
#define TMRA_PWMX_PIN_FUNC              (GPIO_FUNC_4)
#define TMRA_PWMY_PORT                  (GPIO_PORT_A)
#define TMRA_PWMY_PIN                   (GPIO_PIN_09)
#define TMRA_PWMY_PIN_FUNC              (GPIO_FUNC_4)

#define TMRA_MD                         (TMRA_MD_TRIANGLE)
#define TMRA_DIR                        (TMRA_DIR_UP)
#define TMRA_PERIOD_VAL                 (80U)
#define TMRA_PWMX_CMP_VAL               (40U)
#define TMRA_PWMY_CMP_VAL               (48U)

#else
#error "The function is NOT supported."
#endif

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void TmrAConfig(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timera_pwm project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Configures TimerA. */
    TmrAConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Starts TimerA. */
    TMRA_Start(TMRA_UNIT);

    /***************** Configuration end, application start **************/
    for (;;) {
        /*
         * Stop PWM output:
         *   TMRA_Stop(TMRA_UNIT);
         *   or
         *   TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_PWM_x_CH, DISABLE);
         */
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
    stc_tmra_pwm_init_t stcPwmInit;

    /* 1. Enable TimerA peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMRA_PERIPH_CLK, ENABLE);

    /* 2. Set a default initialization value for stcTmraInit. */
    (void)TMRA_StructInit(&stcTmraInit);

    /* 3. Modifies the initialization values depends on the application. */
    stcTmraInit.sw_count.u16CountMode = TMRA_MD;
    stcTmraInit.sw_count.u16CountDir  = TMRA_DIR;
    stcTmraInit.u32PeriodValue = TMRA_PERIOD_VAL;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);

    /* 4. Set the comparison reference value. */
#if (APP_FUNC == APP_FUNC_NORMAL_SINGLE_PWM)
    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = TMRA_PWM_CMP_VAL;
    GPIO_SetFunc(TMRA_PWM_PORT, TMRA_PWM_PIN, TMRA_PWM_PIN_FUNC);
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_PWM_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_PWM_CH, ENABLE);

#elif (APP_FUNC == APP_FUNC_SINGLE_EDGE_ALIGNED_PWM)
    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = TMRA_PWMX_CMP_VAL;
    GPIO_SetFunc(TMRA_PWMX_PORT, TMRA_PWMX_PIN, TMRA_PWMX_PIN_FUNC);
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_PWMX_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_PWMX_CH, ENABLE);

    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = TMRA_PWMY_CMP_VAL;
    GPIO_SetFunc(TMRA_PWMY_PORT, TMRA_PWMY_PIN, TMRA_PWMY_PIN_FUNC);
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_PWMY_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_PWMY_CH, ENABLE);

#elif (APP_FUNC == APP_FUNC_TOW_EDGE_SYMMETRIC_PWM)
    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue        = TMRA_PWMX_CMP_VAL;
    stcPwmInit.u16StartPolarity       = TMRA_PWM_HIGH;
    stcPwmInit.u16StopPolarity        = TMRA_PWM_HIGH;
    stcPwmInit.u16PeriodMatchPolarity = TMRA_PWM_HOLD;
    GPIO_SetFunc(TMRA_PWMX_PORT, TMRA_PWMX_PIN, TMRA_PWMX_PIN_FUNC);
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_PWMX_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_PWMX_CH, ENABLE);

    stcPwmInit.u32CompareValue  = TMRA_PWMY_CMP_VAL;
    stcPwmInit.u16StartPolarity = TMRA_PWM_LOW;
    stcPwmInit.u16StopPolarity  = TMRA_PWM_LOW;
    GPIO_SetFunc(TMRA_PWMY_PORT, TMRA_PWMY_PIN, TMRA_PWMY_PIN_FUNC);
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_PWMY_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_PWMY_CH, ENABLE);
#endif
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
