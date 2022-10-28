/**
 *******************************************************************************
 * @file  timer4/timer4_pwm_dead_timer/source/main.c
 * @brief This example demonstrates how to use the dead timer mode of TMR4 PWM.
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
 * @addtogroup TIMER4_PWM_Dead_Timer
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* TIM4 PWM pin definition */
#define TIM4_OXH_PORT                   (GPIO_PORT_E)
#define TIM4_OXH_PIN                    (GPIO_PIN_09)
#define TIM4_OXH_GPIO_FUNC              (GPIO_FUNC_2)

#define TIM4_OXL_PORT                   (GPIO_PORT_E)
#define TIM4_OXL_PIN                    (GPIO_PIN_08)
#define TIM4_OXL_GPIO_FUNC              (GPIO_FUNC_2)

/* TMR4 unit definition */
#define TMR4_UNIT                       (CM_TMR4_1)
#define TMR4_FCG_ENABLE()               (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR4_1, ENABLE))

/* TMR4 count period value(31.25ms) */
#define TMR4_CNT_PERIOD_VALUE(div)      ((uint16_t)(TMR4_ClockFreq() / (1UL << (uint32_t)(div)) / 32UL) - 1U)

/* TMR4 output-compare channel definition */
#define TMR4_OC_CH                      (TMR4_OC_CH_UL)

/* Get TMR4 PWM channel by OC channel */
#define TMR4_PWM_CH(x)                  (((x) <= TMR4_OC_CH_UL) ? TMR4_PWM_CH_U : \
                                        (((x) <= TMR4_OC_CH_VL) ? TMR4_PWM_CH_V : TMR4_PWM_CH_W))

/* Get TMR4 PWM port by PWM channel */
#define TMR4_PWM_PIN_OXH(x)             ((TMR4_PWM_CH_U == (x)) ? TMR4_PWM_PIN_OUH : \
                                        ((TMR4_PWM_CH_V == (x)) ? TMR4_PWM_PIN_OVH : TMR4_PWM_PIN_OWH))
#define TMR4_PWM_PIN_OXL(x)             ((TMR4_PWM_CH_U == (x)) ? TMR4_PWM_PIN_OUL : \
                                        ((TMR4_PWM_CH_V == (x)) ? TMR4_PWM_PIN_OVL : TMR4_PWM_PIN_OWL))

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

/**
 * @brief  Get TMR4 clock frequency.
 * @param  None
 * @retval TMR4 clock frequency
 */
static uint32_t TMR4_ClockFreq(void)
{
    return CLK_GetBusClockFreq(CLK_BUS_PCLK1);
}

/**
 * @brief  Main function of TMR4 PWM dead timer mode
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t u32PwmCh;
    stc_tmr4_init_t stcTmr4Init;
    stc_tmr4_oc_init_t stcTmr4OcInit;
    stc_tmr4_pwm_init_t stcTmr4PwmInit;
    un_tmr4_oc_ocmrl_t unTmr4OcOcmrl;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize PWM I/O */
    GPIO_SetFunc(TIM4_OXH_PORT, TIM4_OXH_PIN, TIM4_OXH_GPIO_FUNC);
    GPIO_SetFunc(TIM4_OXL_PORT, TIM4_OXL_PIN, TIM4_OXL_GPIO_FUNC);

    /* Enable TMR4 peripheral clock */
    TMR4_FCG_ENABLE();

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    /************************* Configure TMR4 counter *************************/
    (void)TMR4_StructInit(&stcTmr4Init);
    stcTmr4Init.u16ClockDiv = TMR4_CLK_DIV128;
    stcTmr4Init.u16PeriodValue = TMR4_CNT_PERIOD_VALUE(stcTmr4Init.u16ClockDiv);
    (void)TMR4_Init(TMR4_UNIT, &stcTmr4Init);

    /************************* Configure TMR4 output-compare ******************/
    /* TMR4 OC low channel: initialize */
    (void)TMR4_OC_StructInit(&stcTmr4OcInit);
    stcTmr4OcInit.u16CompareValue = stcTmr4Init.u16PeriodValue / 2U;
    (void)TMR4_OC_Init(TMR4_UNIT, TMR4_OC_CH, &stcTmr4OcInit);

    /* TMR4 OCO low channel: compare mode OCMR[31:0] 0x0FF0 0FFF = b 0000 1111 1111 0000   0000 1111 1111 1111 */
    unTmr4OcOcmrl.OCMRx_f.OCFDCL  = TMR4_OC_OCF_SET; /* bit[0]     1  */
    unTmr4OcOcmrl.OCMRx_f.OCFPKL  = TMR4_OC_OCF_SET; /* bit[1]     1  */
    unTmr4OcOcmrl.OCMRx_f.OCFUCL  = TMR4_OC_OCF_SET; /* bit[2]     1  */
    unTmr4OcOcmrl.OCMRx_f.OCFZRL  = TMR4_OC_OCF_SET; /* bit[3]     1  */
    unTmr4OcOcmrl.OCMRx_f.OPDCL   = TMR4_OC_INVT;    /* bit[5:4]   11 */
    unTmr4OcOcmrl.OCMRx_f.OPPKL   = TMR4_OC_INVT;    /* bit[7:6]   11 */
    unTmr4OcOcmrl.OCMRx_f.OPUCL   = TMR4_OC_INVT;    /* bit[9:8]   11 */
    unTmr4OcOcmrl.OCMRx_f.OPZRL   = TMR4_OC_INVT;    /* bit[11:10] 11 */
    unTmr4OcOcmrl.OCMRx_f.OPNPKL  = TMR4_OC_HOLD;    /* bit[13:12] 00 */
    unTmr4OcOcmrl.OCMRx_f.OPNZRL  = TMR4_OC_HOLD;    /* bit[15:14] 00 */
    unTmr4OcOcmrl.OCMRx_f.EOPNDCL = TMR4_OC_HOLD;    /* bit[17:16] 00 */
    unTmr4OcOcmrl.OCMRx_f.EOPNUCL = TMR4_OC_HOLD;    /* bit[19:18] 00 */
    unTmr4OcOcmrl.OCMRx_f.EOPDCL  = TMR4_OC_INVT;    /* bit[21:20] 11 */
    unTmr4OcOcmrl.OCMRx_f.EOPPKL  = TMR4_OC_INVT;    /* bit[23:22] 11 */
    unTmr4OcOcmrl.OCMRx_f.EOPUCL  = TMR4_OC_INVT;    /* bit[25:24] 11 */
    unTmr4OcOcmrl.OCMRx_f.EOPZRL  = TMR4_OC_INVT;    /* bit[27:26] 11 */
    unTmr4OcOcmrl.OCMRx_f.EOPNPKL = TMR4_OC_HOLD;    /* bit[29:28] 00 */
    unTmr4OcOcmrl.OCMRx_f.EOPNZRL = TMR4_OC_HOLD;    /* bit[31:30] 00 */
    TMR4_OC_SetLowChCompareMode(TMR4_UNIT, TMR4_OC_CH, unTmr4OcOcmrl);

    /* TMR4 OC low channel: enable */
    TMR4_OC_Cmd(TMR4_UNIT, TMR4_OC_CH, ENABLE);

    /************************* Configure TMR4 PWM *****************************/
    u32PwmCh = TMR4_PWM_CH(TMR4_OC_CH);

    /* TMR4 PWM: initialize */
    (void)TMR4_PWM_StructInit(&stcTmr4PwmInit);
    stcTmr4PwmInit.u16Mode = TMR4_PWM_MD_DEAD_TMR;
    stcTmr4PwmInit.u16ClockDiv = TMR4_PWM_CLK_DIV128;
    (void)TMR4_PWM_Init(TMR4_UNIT, u32PwmCh, &stcTmr4PwmInit);

    /* TMR4 PWM: set dead time count */
    TMR4_PWM_SetDeadTimeValue(TMR4_UNIT, u32PwmCh, TMR4_PWM_PDAR_IDX, TMR4_OC_GetCompareValue(TMR4_UNIT, TMR4_OC_CH));
    TMR4_PWM_SetDeadTimeValue(TMR4_UNIT, u32PwmCh, TMR4_PWM_PDBR_IDX, 1U);

    /* Start TMR4 count. */
    TMR4_Start(TMR4_UNIT);

    for (;;) {
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
