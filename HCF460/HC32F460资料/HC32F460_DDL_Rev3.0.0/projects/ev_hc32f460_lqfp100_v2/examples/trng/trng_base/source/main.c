/**
 *******************************************************************************
 * @file  trng/trng_base/source/main.c
 * @brief Main program TRNG base for the Device Driver Library.
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
 * @addtogroup TRNG_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Set to non-zero to use TRNG interrupt mode, zero to use polling mode. */
#define TRNG_USE_INTERRUPT      (0U)

#if (TRNG_USE_INTERRUPT > 0U)
/* TRNG interrupt source and number define */
#define TRNG_INT_IRQn           (INT020_IRQn)
#define TRNG_INT_SRC            (INT_SRC_TRNG_END)
#endif

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void TrngConfig(void);
#if (TRNG_USE_INTERRUPT > 0U)
static void TRNG_IrqCallback(void);
#endif

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint32_t m_au32Random[2U];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of template project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Unlock peripherals or registers */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Initializes UART for debug printing. Baudrate is 19200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 19200U, BSP_PRINTF_Preinit);
    /* Config TRNG */
    TrngConfig();
    /* Lock peripherals or registers */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    for (;;) {
#if (TRNG_USE_INTERRUPT > 0U)
        TRNG_Start();
        /* Get random number in TRNG_IrqCallback */
#else
        (void)TRNG_GenerateRandom(m_au32Random, 2U);
#endif
        DDL_DelayMS(200U);
        DDL_Printf("Random numbers: 0x%x, 0x%x\r\n", (unsigned int)m_au32Random[0U], (unsigned int)m_au32Random[1U]);
    }
}

/**
 * @brief  TRNG initialization configuration.
 * @param  None
 * @retval None
 */
static void TrngConfig(void)
{
#if (TRNG_USE_INTERRUPT > 0U)
    stc_irq_signin_config_t stcIrqRegiCfg;
#endif

    /* Enable TRNG. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_TRNG, ENABLE);
    /* TRNG initialization configuration. */
    TRNG_Init(TRNG_SHIFT_CNT64, TRNG_RELOAD_INIT_VAL_ENABLE);

#if (TRNG_USE_INTERRUPT > 0U)
    /* Register IRQ handler && configure NVIC. */
    stcIrqRegiCfg.enIRQn      = TRNG_INT_IRQn;
    stcIrqRegiCfg.enIntSrc    = TRNG_INT_SRC;
    stcIrqRegiCfg.pfnCallback = &TRNG_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqRegiCfg);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
#endif
}

#if (TRNG_USE_INTERRUPT > 0U)
/**
 * @brief  TRNG end IRQ callback
 * @param  None
 * @retval None
 */
void TRNG_IrqCallback(void)
{
    (void)TRNG_GetRandom(m_au32Random, 2U);
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
