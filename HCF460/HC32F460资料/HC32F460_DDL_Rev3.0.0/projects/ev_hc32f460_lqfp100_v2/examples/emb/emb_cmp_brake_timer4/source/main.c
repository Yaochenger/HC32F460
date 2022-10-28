/**
 *******************************************************************************
 * @file  emb/emb_cmp_brake_timer4/source/main.c
 * @brief This example demonstrates how to use CMP brake function of EMB
 *        function.
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
 * @addtogroup EMB_CMP_Brake_TMR4
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

/* CMP pin definition */
#define CMP_INP_PORT                    (GPIO_PORT_A)
#define CMP_INP_PIN                     (GPIO_PIN_04)

/* CMP unit definition */
#define CMP_UNIT                        (CM_CMP2)
#define CMP_FCG_ENABLE()                (FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_CMP, ENABLE))

/* CMP-DAC definition */
#define CMP_8BITDAC_VOLT                (0x80U)
#define CMP_8BITDAC_CH                  (CMP_8BITDAC_CH2)

/* TMR4 PWM pin definition */
#define TIM4_OXH_PORT                   (GPIO_PORT_A)
#define TIM4_OXH_PIN                    (GPIO_PIN_08)
#define TIM4_OXH_GPIO_FUNC              (GPIO_FUNC_2)

#define TIM4_OXL_PORT                   (GPIO_PORT_A)
#define TIM4_OXL_PIN                    (GPIO_PIN_07)
#define TIM4_OXL_GPIO_FUNC              (GPIO_FUNC_2)

/* TMR4 unit definition */
#define TMR4_UNIT                       (CM_TMR4_1)
#define TMR4_FCG_ENABLE()               (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR4_1, ENABLE))

/* TMR4 count period value(250ms) */
#define TMR4_CNT_PERIOD_VALUE(div)      ((uint16_t)(TMR4_ClockFreq() / (1UL << (uint32_t)(div)) / 4UL) - 1U)

/* EMB unit definition */
#define EMB_GROUP                       (CM_EMB1)
#define EMB_FCG_ENABLE()                (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_EMB, ENABLE))

/* EMB interrupt definition */
#define EMB_INT_IRQn                    (INT140_IRQn)
#define EMB_INT_SRC                     (INT_SRC_EMB_GR1)

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
 * @brief  Configure CMP
 * @param  None
 * @retval None
 */
static void CMP_Config(void)
{
    stc_cmp_init_t stcCmpInit;
    stc_gpio_init_t stcGpioInit;

    /* Port function configuration */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(CMP_INP_PORT, CMP_INP_PIN, &stcGpioInit);

    /* Enable CMP peripheral clock */
    CMP_FCG_ENABLE();

    /* De-initialize CMP unit */
    CMP_DeInit(CMP_UNIT);

    /* Config 8 bit DAC for reference voltage */
    CMP_8BitDAC_WriteData(CMP_8BITDAC_CH2, CMP_8BITDAC_VOLT);
    CMP_8BitDAC_Cmd(CMP_8BITDAC_CH, ENABLE);

    /* Configuration for normal compare function */
    (void)CMP_StructInit(&stcCmpInit);
    stcCmpInit.u16PositiveInput = CMP2_POSITIVE_CMP2_INP1;
    stcCmpInit.u16NegativeInput = CMP2_NEGATIVE_DAC2;
    stcCmpInit.u16OutPolarity = CMP_OUT_INVT_OFF;
    stcCmpInit.u16OutDetectEdge = CMP_DETECT_EDGS_BOTH;
    stcCmpInit.u16OutFilter = CMP_OUT_FILTER_CLK_DIV32;
    (void)CMP_NormalModeInit(CMP_UNIT, &stcCmpInit);

    /* Enable CMP output */
    CMP_CompareOutCmd(CMP_UNIT, ENABLE);

    CMP_PinVcoutCmd(CMP_UNIT, ENABLE);
}

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
 * @brief  Configure TMR4 PWM
 * @param  None
 * @retval None
 */
static void TMR4_PwmConfig(void)
{
    stc_tmr4_init_t stcTmr4Init;
    stc_tmr4_oc_init_t stcTmr4OcInit;
    stc_tmr4_pwm_init_t stcTmr4PwmInit;
    un_tmr4_oc_ocmrh_t unTmr4OcOcmrh;
    un_tmr4_oc_ocmrl_t unTmr4OcOcmrl;

    /* Initialize PWM I/O */
    GPIO_SetFunc(TIM4_OXH_PORT, TIM4_OXH_PIN, TIM4_OXH_GPIO_FUNC);
    GPIO_SetFunc(TIM4_OXL_PORT, TIM4_OXL_PIN, TIM4_OXL_GPIO_FUNC);

    /* Enable TMR4 peripheral clock */
    TMR4_FCG_ENABLE();

    /************************* Configure TMR4 counter *************************/
    /* TMR4 counter: initialize */
    (void)TMR4_StructInit(&stcTmr4Init);
    stcTmr4Init.u16ClockDiv = TMR4_CLK_DIV1024;
    stcTmr4Init.u16PeriodValue = TMR4_CNT_PERIOD_VALUE(stcTmr4Init.u16ClockDiv);
    (void)TMR4_Init(TMR4_UNIT, &stcTmr4Init);

    /************************* Configure TMR4 output-compare ******************/
    /* TMR4 OC channel: initialize TMR4 structure */
    (void)TMR4_OC_StructInit(&stcTmr4OcInit);
    stcTmr4OcInit.u16CompareValue = (stcTmr4Init.u16PeriodValue / 2U);

    /* TMR4 OC channel: initialize channel */
    (void)TMR4_OC_Init(TMR4_UNIT, TMR4_OC_CH_UH, &stcTmr4OcInit);
    (void)TMR4_OC_Init(TMR4_UNIT, TMR4_OC_CH_UL, &stcTmr4OcInit);

    /* TMR4 OC high channel: compare mode OCMR[15:0] = 0x0FFF = b 0000 1111 1111 1111 */
    unTmr4OcOcmrh.OCMRx_f.OCFDCH = TMR4_OC_OCF_SET; /* bit[0]     1  */
    unTmr4OcOcmrh.OCMRx_f.OCFPKH = TMR4_OC_OCF_SET; /* bit[1]     1  */
    unTmr4OcOcmrh.OCMRx_f.OCFUCH = TMR4_OC_OCF_SET; /* bit[2]     1  */
    unTmr4OcOcmrh.OCMRx_f.OCFZRH = TMR4_OC_OCF_SET; /* bit[3]     1  */
    unTmr4OcOcmrh.OCMRx_f.OPDCH  = TMR4_OC_INVT;    /* Bit[5:4]   11 */
    unTmr4OcOcmrh.OCMRx_f.OPPKH  = TMR4_OC_INVT;    /* Bit[7:6]   11 */
    unTmr4OcOcmrh.OCMRx_f.OPUCH  = TMR4_OC_INVT;    /* Bit[9:8]   11 */
    unTmr4OcOcmrh.OCMRx_f.OPZRH  = TMR4_OC_INVT;    /* Bit[11:10] 11 */
    unTmr4OcOcmrh.OCMRx_f.OPNPKH = TMR4_OC_HOLD;    /* Bit[13:12] 00 */
    unTmr4OcOcmrh.OCMRx_f.OPNZRH = TMR4_OC_HOLD;    /* Bit[15:14] 00 */
    TMR4_OC_SetHighChCompareMode(TMR4_UNIT, TMR4_OC_CH_UH, unTmr4OcOcmrh);

    /* TMR4 OC low channel: compare mode OCMR[31:0] 0x0FF0 0FFF = b 0000 1111 1111 0000   0000 1111 1111 1111 */
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
    TMR4_OC_SetLowChCompareMode(TMR4_UNIT, TMR4_OC_CH_UL, unTmr4OcOcmrl);

    /* TMR4 OC: enable */
    TMR4_OC_Cmd(TMR4_UNIT, TMR4_OC_CH_UH, ENABLE);
    TMR4_OC_Cmd(TMR4_UNIT, TMR4_OC_CH_UL, ENABLE);

    /************************* Configure TMR4 PWM *****************************/
    /* TMR4 PWM: initialize */
    (void)TMR4_PWM_StructInit(&stcTmr4PwmInit);
    stcTmr4PwmInit.u16Polarity = TMR4_PWM_OXH_HOLD_OXL_INVT;
    (void)TMR4_PWM_Init(TMR4_UNIT, TMR4_PWM_CH_U, &stcTmr4PwmInit);

    /* TMR4 PWM: set PWM pin output when EMB event occur. */
    TMR4_PWM_SetAbnormalPinStatus(TMR4_UNIT, TMR4_PWM_PIN_OUH, TMR4_PWM_ABNORMAL_PIN_LOW);
    TMR4_PWM_SetAbnormalPinStatus(TMR4_UNIT, TMR4_PWM_PIN_OUL, TMR4_PWM_ABNORMAL_PIN_LOW);

    /* Start TMR4 count. */
    TMR4_Start(TMR4_UNIT);
}

/**
 * @brief  EMB IRQ hander.
 * @param  None
 * @retval None
 */
void EMB_GR1_IrqHandler(void)
{
    if (SET == EMB_GetStatus(EMB_GROUP, EMB_FLAG_CMP)) {
        /* Clear the EMB CMP status. */
        EMB_ClearStatus(EMB_GROUP, EMB_FLAG_CMP);
    }
}

/**
 * @brief  Main function of EMB CMP brake
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_emb_tmr4_init_t stcEmbInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Configure CMP. */
    CMP_Config();

    /* Configure TMR4 PWM. */
    TMR4_PwmConfig();

    /* Enable EMB peripheral clock */
    EMB_FCG_ENABLE();

    /* EMB: initialize */
    (void)EMB_TMR4_StructInit(&stcEmbInit);
    stcEmbInit.stcCmp.u32Cmp2State = EMB_CMP2_ENABLE;
    (void)EMB_TMR4_Init(EMB_GROUP, &stcEmbInit);

    /* EMB: enable interrupt */
    EMB_IntCmd(EMB_GROUP, EMB_INT_CMP, ENABLE);

    /* EMB: register IRQ handler && configure NVIC. */
    (void)INTC_ShareIrqCmd(EMB_INT_SRC, ENABLE);
    NVIC_ClearPendingIRQ(EMB_INT_IRQn);
    NVIC_SetPriority(EMB_INT_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(EMB_INT_IRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

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
