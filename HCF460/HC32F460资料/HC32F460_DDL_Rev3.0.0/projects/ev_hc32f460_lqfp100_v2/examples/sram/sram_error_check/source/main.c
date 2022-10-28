/**
 *******************************************************************************
 * @file  sram/sram_error_check/source/main.c
 * @brief Main program of SRAM for the Device Driver Library.
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
 * @addtogroup SRAM_Error_Check
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Function of SRAM checking definitions. */
#define SAMPLE_FUNC_SRAM_PARITY_CHECK   (1U)
#define SAMPLE_FUNC_SRAM_ECC_CHECK      (2U)

/* Select a function of SRAM checking */
#define SAMPLE_FUNC             (SAMPLE_FUNC_SRAM_PARITY_CHECK)

/* Definitions according to the function that just specified. */
#if (SAMPLE_FUNC == SAMPLE_FUNC_SRAM_PARITY_CHECK)
#define SRAM_SEL                (SRAM_SRAM12 | SRAM_SRAMR | SRAM_SRAMH)
#define SRAM_CHECK_ADDR         (0x20010000UL)
#define SRAM_NMI_SRC            (NMI_SRC_SRAM_PARITY)

#else /* (SAMPLE_FUNC == SAMPLE_FUNC_SRAM_ECC_CHECK) */
#define SRAM_SEL                (SRAM_SRAM3)
#define SRAM_CHECK_ADDR         (0x20020000UL)
#define SRAM_ECC_MD             (SRAM_ECC_MD3)
#define SRAM_NMI_SRC            (NMI_SRC_SRAM_ECC)
#endif

/* SRAM error mode. Operation after error occurs. @ref SRAM_Err_Mode */
#define SRAM_ERR_MD             (SRAM_ERR_MD_NMI)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void SramConfig(void);
/* Just for test */
static void SramMakeError(void);

#if (SRAM_ERR_MD == SRAM_ERR_MD_NMI)
static void NMI_SramError_IrqHandler(void);
#endif

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of SRAM error check project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_SRAM);
    /* BSP led */
    BSP_LED_Init();
    /* BSP Key */
    BSP_KEY_Init();
    /* SRAM configuration. */
    SramConfig();
    /* Make error for SRAM error-check testing. */
    SramMakeError();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_SRAM);

    for (;;) {
    }
}

/**
 * @brief  NMI IRQ Handler.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
    NMI_SramError_IrqHandler();
}

/**
 * @brief  Configures SRAM.
 * @param  None
 * @retval None
 */
static void SramConfig(void)
{
    stc_nmi_init_t stcNmiInit;

    SRAM_Init();

    SRAM_SetErrorMode(SRAM_SEL, SRAM_ERR_MD);

#if (SAMPLE_FUNC == SAMPLE_FUNC_SRAM_ECC_CHECK)
    SRAM_SetEccMode(SRAM_SEL, SRAM_ECC_MD);
#endif

    /* NMI interrupt configuration. */
    NMI_StructInit(&stcNmiInit);
    stcNmiInit.u32Src = SRAM_NMI_SRC;
    (void)NMI_Init(&stcNmiInit);
}

/**
 * @brief  Make an error of SRAM.
 * @param  None
 * @retval None
 */
static void SramMakeError(void)
{
    __UNUSED uint32_t u32Tmp;

    BSP_LED_On(LED_BLUE);
    BSP_LED_Off(LED_RED);

    while (BSP_KEY_GetStatus(BSP_KEY_1) != SET) {
        __NOP();
    }

#if (SAMPLE_FUNC == SAMPLE_FUNC_SRAM_PARITY_CHECK)
    /* Read a SRAM address that uninitialized and the parity check error will occur after the reading operation. */
    u32Tmp = RW_MEM32(SRAM_CHECK_ADDR);
#else
    /* 1. Specifies an ECC mode. */
    SRAM_SetEccMode(SRAM_SEL, SRAM_ECC_MD);

    /* 2. Write and read while an ECC is enabled. */
    RW_MEM32(SRAM_CHECK_ADDR) = 0x12345678UL;
    u32Tmp = RW_MEM32(SRAM_CHECK_ADDR);

    /* 3. Disable ECC mode and write a different value to the same address. */
    SRAM_SetEccMode(SRAM_SEL, SRAM_ECC_MD_INVD);
    RW_MEM32(SRAM_CHECK_ADDR) = 0x11223344UL;

    /* 4. Enable the ECC mode again. */
    SRAM_SetEccMode(SRAM_SEL, SRAM_ECC_MD);

    /* 5. Read the address that was just written and the ECC check error will occur after the reading operation. */
    u32Tmp = RW_MEM32(SRAM_CHECK_ADDR);
#endif
}

/**
 * @brief  NMI SRAM error IRQ handler.
 * @param  None
 * @retval None
 */
static void NMI_SramError_IrqHandler(void)
{
    SRAM_ClearStatus(SRAM_FLAG_ALL);
    NMI_ClearNmiStatus(NMI_SRC_SRAM_PARITY | NMI_SRC_SRAM_ECC);
    /* Turn on the red led for indicating. */
    BSP_LED_On(LED_RED);
    BSP_LED_Off(LED_BLUE);
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
