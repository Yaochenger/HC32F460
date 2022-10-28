/**
 *******************************************************************************
 * @file  mpu/mpu_dma_write_protect/source/main.c
 * @brief Main program of MPU dma write protect for the Device Driver Library.
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
 * @addtogroup MPU_DMA_Write_Protect
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM | LL_PERIPH_MPU)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

/* MPU DMA configuration define */
#define MPU_DMA_UNIT                    (CM_DMA2)
#define MPU_DMA_CLK                     (FCG0_PERIPH_DMA2 | FCG0_PERIPH_AOS)
#define MPU_DMA_CH                      (DMA_CH0)
#define MPU_DMA_TRIG_CH                 (AOS_DMA2_0)
#define MPU_DMA_INT_CH                  (DMA_INT_TC_CH0)
#define MPU_DMA_INT_SRC                 (INT_SRC_DMA2_TC0)
#define MPU_DMA_IRQn                    (INT006_IRQn)

#define MPU_UNIT                        (MPU_UNIT_DMA2)
#define MPU_REGION                      (MPU_REGION_NUM2)
#define MPU_DMA_BUF_SIZE                (256U)
#define MPU_DMA_TRANS_CNT               (1U)

/* Address alignment define */
#if defined ( __GNUC__ ) && !defined (__CC_ARM) /*!< GNU Compiler */
#define __ALIGN_MPU_1024                __attribute__((aligned(1024)))
#elif defined (__ICCARM__)                /*!< IAR Compiler */
#define __ALIGN_MPU_1024                _Pragma("data_alignment=1024")
#elif defined (__CC_ARM)                /*!< ARM Compiler */
#define __ALIGN_MPU_1024                __align(1024)
#else
#error  "unsupported compiler!!"
#endif

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
__ALIGN_MPU_1024 static uint32_t u32SrcBuf[MPU_DMA_BUF_SIZE]  = {0UL};
__ALIGN_MPU_1024 static uint32_t u32DestBuf[MPU_DMA_BUF_SIZE] = {0UL};
static uint8_t u8TransCompleteFlag = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  NMI MPU error IRQ handler.
 * @param  None
 * @retval None
 */
static void NMI_MpuError_IrqHandler(void)
{
    if (SET == NMI_GetNmiStatus(NMI_SRC_BUS_ERR)) {
        NMI_ClearNmiStatus(NMI_SRC_BUS_ERR);
        if (SET == MPU_GetStatus(MPU_FLAG_SMPU2EAF)) {
            MPU_ClearStatus(MPU_FLAG_SMPU2EAF);
            BSP_LED_On(LED_RED);
        }
    }
}

/**
 * @brief  DMA transfer complete interrupt callback function.
 * @param  None
 * @retval None
 */
static void DMA_TransCompleteCallBack(void)
{
    u8TransCompleteFlag = 1U;
    DMA_ClearTransCompleteStatus(MPU_DMA_UNIT, MPU_DMA_INT_CH);
}

/**
 * @brief  MPU configuration.
 * @param  None
 * @retval None
 */
static void MPU_Config(void)
{
    stc_mpu_init_t stcMpuInit;
    stc_mpu_region_init_t stcRegionInit;
    stc_nmi_init_t stcNmiInit;

    (void)MPU_StructInit(&stcMpuInit);
    /* Configure MPU */
    stcMpuInit.stcDma2.u32ExceptionType   = MPU_EXP_TYPE_NMI;
    stcMpuInit.stcDma2.u32BackgroundWrite = MPU_BACKGROUND_WR_ENABLE;
    stcMpuInit.stcDma2.u32BackgroundRead  = MPU_BACKGROUND_RD_ENABLE;
    (void)MPU_Init(&stcMpuInit);

    /* Configure the protected region */
    (void)MPU_RegionStructInit(&stcRegionInit);
    stcRegionInit.u32BaseAddr            = (uint32_t)(&u32DestBuf[0]);
    stcRegionInit.u32Size                = MPU_REGION_SIZE_1KBYTE;
    stcRegionInit.stcDma2.u32RegionWrite = MPU_REGION_WR_DISABLE;
    stcRegionInit.stcDma2.u32RegionRead  = MPU_REGION_RD_ENABLE;
    (void)MPU_RegionInit(MPU_REGION, &stcRegionInit);
    MPU_RegionCmd(MPU_REGION, MPU_UNIT, ENABLE);

    /* Configure NMI interrupt */
    (void)NMI_StructInit(&stcNmiInit);
    stcNmiInit.u32Src = NMI_SRC_BUS_ERR;
    (void)NMI_Init(&stcNmiInit);
}

/**
 * @brief  DMA configuration.
 * @param  None
 * @retval None
 */
static void DMA_Config(void)
{
    stc_dma_init_t stcDmaInit;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* MPU DMA configuration */
    FCG_Fcg0PeriphClockCmd(MPU_DMA_CLK, ENABLE);
    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32IntEn         = DMA_INT_ENABLE;
    stcDmaInit.u32DataWidth     = DMA_DATAWIDTH_32BIT;
    stcDmaInit.u32BlockSize     = MPU_DMA_BUF_SIZE;
    stcDmaInit.u32TransCount    = MPU_DMA_TRANS_CNT;
    /* Set source & destination address mode */
    stcDmaInit.u32SrcAddrInc    = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc   = DMA_DEST_ADDR_INC;
    stcDmaInit.u32DestAddr  = (uint32_t)(&u32DestBuf[0]);
    stcDmaInit.u32SrcAddr   = (uint32_t)(&u32SrcBuf[0]);
    if (LL_OK != DMA_Init(MPU_DMA_UNIT, MPU_DMA_CH, &stcDmaInit)) {
        for (;;) {
        }
    }
    AOS_SetTriggerEventSrc(MPU_DMA_TRIG_CH, EVT_SRC_AOS_STRG);

    /* Interrupt configuration */
    stcIrqSignConfig.enIntSrc    = MPU_DMA_INT_SRC;
    stcIrqSignConfig.enIRQn      = MPU_DMA_IRQn;
    stcIrqSignConfig.pfnCallback = &DMA_TransCompleteCallBack;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    /* Enable DMA */
    DMA_Cmd(MPU_DMA_UNIT, ENABLE);
    /* Enable DMA channel */
    DMA_ClearTransCompleteStatus(MPU_DMA_UNIT, MPU_DMA_INT_CH);
    (void)DMA_ChCmd(MPU_DMA_UNIT, MPU_DMA_CH, ENABLE);
}

/**
 * @brief  Init the test buffer.
 * @param  None
 * @retval None
 */
static void TestBufferInit(void)
{
    uint32_t i;

    for (i = 0UL; i < MPU_DMA_BUF_SIZE; i++) {
        u32SrcBuf[i]  = i + 1U;
        u32DestBuf[i] = 0UL;
    }
}

/**
 * @brief  NMI IRQ Handler.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
    NMI_MpuError_IrqHandler();
}

/**
 * @brief  Main function of MPU dma write protect.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t u8ProtectState = 0U;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Configure MPU */
    MPU_Config();
    /* Configure DMA for MPU */
    DMA_Config();
    /* Init test buffer */
    TestBufferInit();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        /* Trigger the DMA */
        if (SET == BSP_KEY_GetStatus(BSP_KEY_3)) {
            AOS_SW_Trigger();
        }
        /* Set the protect state */
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            BSP_LED_Off(LED_RED);
            BSP_LED_Off(LED_BLUE);
            if (0U == u8ProtectState) {
                /* Enable write protection of DMA2 for the region */
                MPU_UnitCmd(MPU_UNIT, ENABLE);
                BSP_LED_On(LED_YELLOW);
                u8ProtectState = 1U;
            } else {
                /* Disable write protection of DMA2 for the region */
                MPU_UnitCmd(MPU_UNIT, DISABLE);
                BSP_LED_Off(LED_YELLOW);
                u8ProtectState = 0U;
            }
        }

        /* Check the result of transfer */
        if (1U == u8TransCompleteFlag) {
            u8TransCompleteFlag = 0U;
            if (0 != memcmp(u32DestBuf, u32SrcBuf, (uint32_t)(MPU_DMA_BUF_SIZE << 2U))) {
                BSP_LED_Off(LED_BLUE);
            } else {
                /* Consistent data content */
                BSP_LED_On(LED_BLUE);
                BSP_LED_Off(LED_RED);
            }
            TestBufferInit();
            /* Reload the configure of the DMA */
            DMA_SetSrcAddr(MPU_DMA_UNIT, MPU_DMA_CH, (uint32_t)(&u32SrcBuf[0]));
            DMA_SetDestAddr(MPU_DMA_UNIT, MPU_DMA_CH, (uint32_t)(&u32DestBuf[0]));
            DMA_SetTransCount(MPU_DMA_UNIT, MPU_DMA_CH, MPU_DMA_TRANS_CNT);
            (void)DMA_ChCmd(MPU_DMA_UNIT, MPU_DMA_CH, ENABLE);
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
