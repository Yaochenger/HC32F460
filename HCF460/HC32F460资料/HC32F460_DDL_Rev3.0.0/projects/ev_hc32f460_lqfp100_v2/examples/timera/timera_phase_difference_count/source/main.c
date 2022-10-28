/**
 *******************************************************************************
 * @file  timera/timera_phase_difference_count/source/main.c
 * @brief Main program TimerA phase difference count for the Device Driver Library.
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
 * @addtogroup TIMERA_Phase_Difference_Count
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Phase-difference count. */
#define PHASE_DIFF_CNT_X1               (1U)
#define PHASE_DIFF_CNT_X2               (2U)
#define PHASE_DIFF_CNT_X4               (4U)

/* Select phase-difference count. */
#define PHASE_DIFF_CNT                  (PHASE_DIFF_CNT_X1)

/* Quadrature Encoder PPR(Pulse Per Round) */
#define QE_PPR                          (1000U)
/* Quadrature Encoder cycles per round. Different Quadrature Encoder different parameter. */
#define QE_CYCLE_PER_ROUND              (QE_PPR * PHASE_DIFF_CNT)

/* TimerA unit and channel definitions for this example. */
#define TMRA_UNIT                       (CM_TMRA_5)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_5)

/* Select the pins for phase A and phase B inputing according to 'TMRA_UNIT'. */
#define PHASE_A_PORT                    (GPIO_PORT_A)
#define PHASE_A_PIN                     (GPIO_PIN_02)
#define PHASE_A_PIN_FUNC                (GPIO_FUNC_5)
#define PHASE_B_PORT                    (GPIO_PORT_A)
#define PHASE_B_PIN                     (GPIO_PIN_03)
#define PHASE_B_PIN_FUNC                (GPIO_FUNC_5)

/* Define the configuration values according to the function that selected. */
#if (PHASE_DIFF_CNT == PHASE_DIFF_CNT_X1)
#define TMRA_CNT_UP_COND                (TMRA_CNT_UP_COND_CLKB_HIGH_CLKA_RISING)
#define TMRA_CNT_DOWN_COND              (TMRA_CNT_DOWN_COND_CLKA_HIGH_CLKB_RISING)

#elif (PHASE_DIFF_CNT == PHASE_DIFF_CNT_X2)
#define TMRA_CNT_UP_COND                (TMRA_CNT_UP_COND_CLKB_HIGH_CLKA_RISING | \
                                         TMRA_CNT_UP_COND_CLKB_LOW_CLKA_FALLING)
#define TMRA_CNT_DOWN_COND              (TMRA_CNT_DOWN_COND_CLKA_HIGH_CLKB_RISING | \
                                         TMRA_CNT_DOWN_COND_CLKA_LOW_CLKB_FALLING)
#elif (PHASE_DIFF_CNT == PHASE_DIFF_CNT_X4)
#define TMRA_CNT_UP_COND                (TMRA_CNT_UP_COND_CLKB_HIGH_CLKA_RISING  | \
                                         TMRA_CNT_UP_COND_CLKB_LOW_CLKA_FALLING  | \
                                         TMRA_CNT_UP_COND_CLKA_HIGH_CLKB_FALLING | \
                                         TMRA_CNT_UP_COND_CLKA_LOW_CLKB_RISING)
#define TMRA_CNT_DOWN_COND              (TMRA_CNT_DOWN_COND_CLKA_HIGH_CLKB_RISING  | \
                                         TMRA_CNT_DOWN_COND_CLKA_LOW_CLKB_FALLING  | \
                                         TMRA_CNT_DOWN_COND_CLKB_HIGH_CLKA_FALLING | \
                                         TMRA_CNT_DOWN_COND_CLKB_LOW_CLKA_RISING)
#else
#error "Phase-difference count is NOT supported!!!"
#endif

/* Timer0 for this example */
#define TMR0_UNIT                       (CM_TMR0_1)
#define TMR0_CH                         (TMR0_CH_B)
#define TMR0_PERIPH_CLK                 (FCG2_PERIPH_TMR0_1)
#define TMR0_INT_TYPE                   (TMR0_INT_CMP_B)
#define TMR0_INT_PRIO                   (DDL_IRQ_PRIO_03)
#define TMR0_INT_SRC                    (INT_SRC_TMR0_1_CMP_B)
#define TMR0_INT_IRQn                   (INT044_IRQn)
#define TMR0_INT_FLAG                   (TMR0_FLAG_CMP_B)

#define TMR0_CMP_VAL                    (39063UL - 1UL)
#define TMR0_CLK_DIV                    (TMR0_CLK_DIV256)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void TmrAConfig(void);

/* 1 second timer, for calculating rotation speed. */
static void Tmr0Config(void);
static void TMR0_IrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
__IO static uint8_t m_u8SpeedUpd = 0U;
static uint32_t m_u32QESpeed = 0UL;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timera_phase_difference_count project
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
    /* Initializes UA_RISINGT for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configures Timer0. */
    Tmr0Config();
    /* Configures TimerA. */
    TmrAConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Starts TimerA. */
    TMRA_Start(TMRA_UNIT);
    /* Starts Timer0. */
    TMR0_Start(TMR0_UNIT, TMR0_CH);

    /***************** Configuration end, application start **************/
    for (;;) {
        if (m_u8SpeedUpd != 0U) {
            DDL_Printf("Quadrature Encoder speed: %u RPM\r\n", (unsigned int)m_u32QESpeed);
            m_u8SpeedUpd = 0U;
        }
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
    stcTmraInit.u8CountSrc = TMRA_CNT_SRC_HW;
    stcTmraInit.hw_count.u16CountUpCond   = TMRA_CNT_UP_COND;
    stcTmraInit.hw_count.u16CountDownCond = TMRA_CNT_DOWN_COND;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);

    /* 4. Configures the function of phase A pin and phase pin. */
    GPIO_SetFunc(PHASE_A_PORT, PHASE_A_PIN, PHASE_A_PIN_FUNC);
    GPIO_SetFunc(PHASE_B_PORT, PHASE_B_PIN, PHASE_B_PIN_FUNC);
}

/**
 * @brief  Timer0 configuration.
 * @param  None
 * @retval None
 */
static void Tmr0Config(void)
{
    stc_tmr0_init_t stcTmr0Init;
    stc_irq_signin_config_t stcIrq;

    /* 1. Enable Timer0 peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMR0_PERIPH_CLK, ENABLE);

    /* 2. Set a default initialization value for stcTmr0Init. */
    (void)TMR0_StructInit(&stcTmr0Init);

    /* 3. Modifies the initialization values depends on the application. */
    stcTmr0Init.u32ClockSrc     = TMR0_CLK_SRC_INTERN_CLK;
    stcTmr0Init.u32ClockDiv     = TMR0_CLK_DIV;
    stcTmr0Init.u32Func         = TMR0_FUNC_CMP;
    stcTmr0Init.u16CompareValue = (uint16_t)TMR0_CMP_VAL;
    (void)TMR0_Init(TMR0_UNIT, TMR0_CH, &stcTmr0Init);

    /* 4. Configures IRQ if needed. */
    stcIrq.enIntSrc    = TMR0_INT_SRC;
    stcIrq.enIRQn      = TMR0_INT_IRQn;
    stcIrq.pfnCallback = &TMR0_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, TMR0_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);
    /* Enable the specified interrupts of Timer0. */
    TMR0_IntCmd(TMR0_UNIT, TMR0_INT_TYPE, ENABLE);
}

/**
 * @brief  Timer0 interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMR0_IrqCallback(void)
{
    uint32_t u32Dir;
    uint32_t u32CurrCycleCnt;
    uint32_t u32CycleCnt;
    static uint32_t u32LastCycleCnt = 0UL;
    static uint32_t u32TmrCnt = 0UL;

    u32TmrCnt++;
    /* 100ms * 10 = 1s */
    if (u32TmrCnt >= 10U) {
        u32CurrCycleCnt = TMRA_GetCountValue(TMRA_UNIT);
        u32Dir = TMRA_GetCountDir(TMRA_UNIT);

        if (u32Dir == TMRA_DIR_DOWN) {
            if (u32CurrCycleCnt > u32LastCycleCnt) {
                u32CycleCnt = (65536UL + u32LastCycleCnt) - u32CurrCycleCnt;
            } else {
                u32CycleCnt = u32LastCycleCnt - u32CurrCycleCnt;
            }
        } else {
            /* (u32TmrADir == TMRA_DIR_UP) */
            if (u32CurrCycleCnt > u32LastCycleCnt) {
                u32CycleCnt = u32CurrCycleCnt - u32LastCycleCnt;
            } else if (u32CurrCycleCnt < u32LastCycleCnt) {
                u32CycleCnt = (65536UL + u32CurrCycleCnt) - u32LastCycleCnt;
            } else {
                u32CycleCnt = u32LastCycleCnt - u32CurrCycleCnt;
            }
        }

        m_u32QESpeed    = (u32CycleCnt * 60UL) / QE_CYCLE_PER_ROUND;
        u32LastCycleCnt = u32CurrCycleCnt;
        m_u8SpeedUpd    = 1U;
        u32TmrCnt       = 0UL;
    }
    TMR0_ClearStatus(TMR0_UNIT, TMR0_INT_FLAG);
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
