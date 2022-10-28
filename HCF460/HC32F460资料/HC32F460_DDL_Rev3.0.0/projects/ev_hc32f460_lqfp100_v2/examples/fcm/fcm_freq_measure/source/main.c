/**
 *******************************************************************************
 * @file  fcm/fcm_freq_measure/source/main.c
 * @brief FCM main program example for the Device Driver Library.
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
 * @addtogroup FCM
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
typedef struct {
    char *pi8TargetClock;
    uint32_t u32TargetClock;
    uint32_t u32TargetClockFreq;
} stcFmcTargetTbl_t;

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define REF_FREQ            (XTAL_VALUE)
#define REF_DIV             (8192U)
#define TAR_DIV             (1U)
#define FMC_DEMO_BAUDRATE   (9600U)
#define XTAL32_FREQ         (XTAL32_VALUE)
#define HRC_FREQ            (16UL * 1000UL * 1000UL)
#define LRC_FREQ            (LRC_VALUE)
#define SWDTLRC_FREQ        (SWDTLRC_VALUE)
#define PCLK1_FREQ          (4UL * 1000UL * 1000UL)
#define UPLLP_FREQ          (50UL * 1000UL * 1000UL)
#define MRC_FREQ            (MRC_VALUE)
#define MPLLP_FREQ          (25UL * 1000UL * 1000UL)

#define FCM_ERR_IRQn        (INT000_IRQn)
#define FCM_OVF_IRQn        (INT001_IRQn)
#define FCM_END_IRQn        (INT002_IRQn)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void FCM_Error_IrqCallback(void);
static void FCM_Ovf_IrqCallback(void);
static void FCM_End_IrqCallback(void);
static void FcmErrorIntInit(void);
static void FcmEndIntInit(void);
static void FcmOvfIntInit(void);
static void FcmInit(uint32_t u32TargetClock, uint32_t u32TargetClockFreq);
static void RefClockInit(void);
static void TargetClockInit(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static char *m_pi8TargetClock;
static stcFmcTargetTbl_t m_stcFcmTargetTbl[] = {
    {"1.HRC", FCM_TARGET_CLK_HRC, HRC_FREQ},
    {"2.LRC", FCM_TARGET_CLK_LRC, LRC_FREQ},
    {"3.SWDTLRC", FCM_TARGET_CLK_SWDTLRC, SWDTLRC_FREQ},
    {"4.PCLK1(SYSCLK=8MHz div2)", FCM_TARGET_CLK_PCLK1, PCLK1_FREQ},
    {"5.MRC", FCM_TARGET_CLK_MRC, MRC_VALUE},
    {"6.MPLLP(VCO=400MHz div16)", FCM_TARGET_CLK_MPLLP, MPLLP_FREQ},
    {"7.UPLLP(VCO=400MHz div8)", FCM_TARGET_CLK_UPLLP, UPLLP_FREQ},
    {"8.XTAL32", FCM_TARGET_CLK_XTAL32, XTAL32_FREQ},
};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  FCM frequency error IRQ callback
 * @param  None
 * @retval None
 */
static void FCM_Error_IrqCallback(void)
{
    DDL_Printf("Error: Freq. out of range!\r\n");
    FCM_Cmd(DISABLE);
    FCM_ClearStatus(FCM_FLAG_ERR);
}

/**
 * @brief  FCM measure counter overflow IRQ callback
 * @param  None
 * @retval None
 */
static void FCM_Ovf_IrqCallback(void)
{
    DDL_Printf("Error: Count Overflow!\r\n");
    FCM_Cmd(DISABLE);
    FCM_ClearStatus(FCM_FLAG_OVF);
}

/**
 * @brief  FCM measure end IRQ callback
 * @param  None
 * @retval None
 */
static void FCM_End_IrqCallback(void)
{
    uint16_t u16FcmCnt;
    uint32_t u32TargetClock;
    u16FcmCnt = FCM_GetCountValue();
    FCM_Cmd(DISABLE);
    FCM_ClearStatus(FCM_FLAG_END);

    u32TargetClock = READ_REG32_BIT(CM_FCM->MCCR, FCM_MCCR_MCKS);
    if ((FCM_TARGET_CLK_XTAL32 == u32TargetClock) || (FCM_TARGET_CLK_LRC == u32TargetClock) || \
            (FCM_TARGET_CLK_SWDTLRC == u32TargetClock)) {
        DDL_Printf("%s freq. is %lu Hz\r\n", m_pi8TargetClock, (REF_FREQ * u16FcmCnt / REF_DIV) * TAR_DIV);
    } else {
        DDL_Printf("%s freq. is %lu KHz\r\n", m_pi8TargetClock, (REF_FREQ / 1000UL * u16FcmCnt / REF_DIV) * TAR_DIV);
    }
}

/**
 * @brief  FCM frequency error interrupt init
 * @param  None
 * @retval None
 */
static void FcmErrorIntInit(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;

    stcIrqSignConfig.enIntSrc = INT_SRC_FCMFERRI;
    stcIrqSignConfig.enIRQn   = FCM_ERR_IRQn;
    stcIrqSignConfig.pfnCallback = &FCM_Error_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    /* NVIC config */
    NVIC_ClearPendingIRQ(FCM_ERR_IRQn);
    NVIC_SetPriority(FCM_ERR_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(FCM_ERR_IRQn);
}

/**
 * @brief  FCM measure end interrupt init
 * @param  None
 * @retval None
 */
static void FcmEndIntInit(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;

    stcIrqSignConfig.enIntSrc = INT_SRC_FCMMENDI;
    stcIrqSignConfig.enIRQn   = FCM_END_IRQn;
    stcIrqSignConfig.pfnCallback = &FCM_End_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    /* NVIC config */
    NVIC_ClearPendingIRQ(FCM_END_IRQn);
    NVIC_SetPriority(FCM_END_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(FCM_END_IRQn);
}

/**
 * @brief  FCM counter overflow interrupt init
 * @param  None
 * @retval None
 */
static void FcmOvfIntInit(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;

    stcIrqSignConfig.enIntSrc = INT_SRC_FCMCOVFI;
    stcIrqSignConfig.enIRQn   = FCM_OVF_IRQn;
    stcIrqSignConfig.pfnCallback = &FCM_Ovf_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    /* NVIC config */
    NVIC_ClearPendingIRQ(FCM_OVF_IRQn);
    NVIC_SetPriority(FCM_OVF_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(FCM_OVF_IRQn);
}

/**
 * @brief  FCM init
 * @param  [in] u32TargetClock Target clock type selection.
 *   @arg  FCM_REF_CLK_EXINPUT
 *   @arg  FCM_TARGET_CLK_XTAL
 *   @arg  FCM_TARGET_CLK_XTAL32
 *   @arg  FCM_TARGET_CLK_HRC
 *   @arg  FCM_TARGET_CLK_LRC
 *   @arg  FCM_TARGET_CLK_SWDTLRC
 *   @arg  FCM_TARGET_CLK_PCLK1
 *   @arg  FCM_TARGET_CLK_UPLLP
 *   @arg  FCM_TARGET_CLK_MRC
 *   @arg  FCM_TARGET_CLK_MPLLP
 * @param  [in] u32TargetClockFreq Target clock frequency.
 * @retval None
 */
static void FcmInit(uint32_t u32TargetClock, uint32_t u32TargetClockFreq)
{
    stc_fcm_init_t stcFcmInit;

    (void)FCM_StructInit(&stcFcmInit);
    stcFcmInit.u32RefClock     = FCM_REF_CLK_XTAL;
    stcFcmInit.u32RefClockDiv  = FCM_REF_CLK_DIV8192;
    stcFcmInit.u32RefClockEdge = FCM_REF_CLK_RISING;
    stcFcmInit.u32TargetClock  = u32TargetClock;
    stcFcmInit.u32ExceptionType = FCM_EXP_TYPE_INT;
    stcFcmInit.u32TargetClockDiv = FCM_TARGET_CLK_DIV1;

    /* idea count value = (tar_freq/tar_div)/(ref_freq/ref_div) */
    stcFcmInit.u16LowerLimit = (uint16_t)((((u32TargetClockFreq / TAR_DIV) / (REF_FREQ / REF_DIV)) * 97UL) / 100UL);
    stcFcmInit.u16UpperLimit = (uint16_t)((((u32TargetClockFreq / TAR_DIV) / (REF_FREQ / REF_DIV)) * 103UL) / 100UL);
    if (1U == (stcFcmInit.u16UpperLimit - stcFcmInit.u16LowerLimit)) {
        stcFcmInit.u16LowerLimit -= 1U;
        stcFcmInit.u16UpperLimit += 1U;
    }

    (void)FCM_Init(&stcFcmInit);
    FCM_IntCmd((FCM_INT_OVF | FCM_INT_END | FCM_INT_ERR), ENABLE);
}

/**
 * @brief  FCM reference clock init
 * @param  None
 * @retval None
 */
static void RefClockInit(void)
{
    stc_clock_xtal_init_t stcXtalInit;

    (void)CLK_XtalStructInit(&stcXtalInit);
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8Mode  = CLK_XTAL_MD_OSC;
    stcXtalInit.u8Drv   = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_XTAL);
}

/**
 * @brief  FCM target clock init, including HRC, MRC, LRC, MPLL, UPLL, XTAL32
 * @param  None
 * @retval None
 */
static void TargetClockInit(void)
{
    stc_clock_xtal32_init_t stcXtal32Init;
    stc_clock_pll_init_t stcMPLLInit;
    stc_clock_pllx_init_t stcUPLLInit;

    /* Xtal32 config */
    (void)CLK_Xtal32StructInit(&stcXtal32Init);
    stcXtal32Init.u8State = CLK_XTAL32_ON;
    stcXtal32Init.u8Drv   = CLK_XTAL32_DRV_MID;
    stcXtal32Init.u8Filter = CLK_XTAL32_FILTER_ALL_MD;
    (void)CLK_Xtal32Init(&stcXtal32Init);

    (void)CLK_HrcCmd(ENABLE);
    (void)CLK_MrcCmd(ENABLE);
    (void)CLK_LrcCmd(ENABLE);

    /* PCLK0, HCLK  Max 200MHz */
    /* PCLK1, PCLK4 Max 100MHz */
    /* PCLK2, PCLK3 Max 60MHz  */
    /* EX BUS Max 100MHz */
    CLK_SetClockDiv(CLK_BUS_CLK_ALL,                                                   \
                    (CLK_PCLK0_DIV1 | CLK_PCLK1_DIV2 | CLK_PCLK2_DIV4 |             \
                     CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2 | CLK_EXCLK_DIV2 |             \
                     CLK_HCLK_DIV1));

    /* PLLH config */
    (void)CLK_PLLStructInit(&stcMPLLInit);
    /*
        VCO = 8/1*50 = 400MHz
        8MHz/M*N = 8/1*50/2 =200MHz
    */
    stcMPLLInit.u8PLLState = CLK_PLLX_ON;
    stcMPLLInit.PLLCFGR = 0UL;
    stcMPLLInit.PLLCFGR_f.PLLM = (1UL  - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLN = (50UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLR = (2UL  - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLQ = (2UL  - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLP = (16UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;     /* Xtal = 8MHz */
    (void)CLK_PLLInit(&stcMPLLInit);

    /* PLLA config */
    (void)CLK_PLLxStructInit(&stcUPLLInit);
    /*
        VCO = 8/2*100 = 400MHz
        8MHz/M*N = 8/2*100/2 =200MHz
    */
    stcUPLLInit.u8PLLState = CLK_PLLX_ON;
    stcUPLLInit.PLLCFGR = 0UL;
    stcUPLLInit.PLLCFGR_f.PLLM = (2UL  - 1UL);
    stcUPLLInit.PLLCFGR_f.PLLN = (100UL - 1UL);
    stcUPLLInit.PLLCFGR_f.PLLR = (2UL  - 1UL);
    stcUPLLInit.PLLCFGR_f.PLLQ = (2UL  - 1UL);
    stcUPLLInit.PLLCFGR_f.PLLP = (8UL  - 1UL);

    (void)CLK_PLLxInit(&stcUPLLInit);
}

/**
 * @brief  Wait Key K10 press
 * @param  None
 * @retval None
 */
static void WaitKeyPress(void)
{
    while (SET != BSP_KEY_GetStatus(BSP_KEY_10)) {
        DDL_DelayMS(10UL);
        SWDT_FeedDog();
    }
    while (RESET != BSP_KEY_GetStatus(BSP_KEY_10)) {
        DDL_DelayMS(10UL);
        SWDT_FeedDog();
    }
}

/**
 * @brief  Main function of FCM project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t i = 0U;
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU);
    /* BSP key init */
    BSP_KEY_Init();

    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_FCM, ENABLE);

    RefClockInit();
    TargetClockInit();
    FcmErrorIntInit();
    FcmEndIntInit();
    FcmOvfIntInit();

    DDL_PrintfInit(BSP_PRINTF_DEVICE, FMC_DEMO_BAUDRATE, BSP_PRINTF_Preinit);

    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_FCG | LL_PERIPH_GPIO);

    DDL_Printf("XTAL=8MHz divided by 8192 is used as the reference clock for this demo.\r\n");

    for (;;) {
        WaitKeyPress();
        m_pi8TargetClock = m_stcFcmTargetTbl[i].pi8TargetClock;
        FcmInit(m_stcFcmTargetTbl[i].u32TargetClock, m_stcFcmTargetTbl[i].u32TargetClockFreq);
        FCM_Cmd(ENABLE);
        i++;
        if (i >= sizeof(m_stcFcmTargetTbl) / sizeof(m_stcFcmTargetTbl[0])) {
            i = 0;
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
