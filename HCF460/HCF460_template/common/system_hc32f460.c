/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file system_hc32f460.c
 **
 ** A detailed description is available at
 ** @link Hc32f460SystemGroup Hc32f460System description @endlink
 **
 **   - 2018-10-15 CDT First version
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_common.h"
#include "hc32_ddl.h"
/**
 *******************************************************************************
 ** \addtogroup Hc32f460SystemGroup
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('define')
 ******************************************************************************/

//@{

/**
 ******************************************************************************
 ** System Clock Frequency (Core Clock) Variable according CMSIS
 ******************************************************************************/
uint32_t HRC_VALUE = HRC_16MHz_VALUE;
uint32_t SystemCoreClock = MRC_VALUE;

/**
 ******************************************************************************
 ** \brief  Setup the microcontroller system. Initialize the System and update
 ** the SystemCoreClock variable.
 **
 ** \param  None
 ** \return None
 ******************************************************************************/
void SystemInit(void)
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20) | (3UL << 22)); /* set CP10 and CP11 Full Access */
#endif

    SystemCoreClockUpdate();
}

void SystemCoreClockUpdate(void)  // Update SystemCoreClock variable
{
    uint8_t tmp = 0u;
    uint32_t plln = 19u, pllp = 1u, pllm = 0u, pllsource = 0u;

    /* Select proper HRC_VALUE according to ICG1.HRCFREQSEL bit */
    /* ICG1.HRCFREQSEL = '0' represent HRC_VALUE = 20000000UL   */
    /* ICG1.HRCFREQSEL = '1' represent HRC_VALUE = 16000000UL   */
    if (1UL == (HRC_FREQ_MON() & 1UL))
    {
        HRC_VALUE = HRC_16MHz_VALUE;
    }
    else
    {
        HRC_VALUE = HRC_20MHz_VALUE;
    }

    tmp = M4_SYSREG->CMU_CKSWR_f.CKSW;
    switch (tmp)
    {
        case 0x00:  /* use internal high speed RC */
            SystemCoreClock = HRC_VALUE;
            break;
        case 0x01:  /* use internal middle speed RC */
            SystemCoreClock = MRC_VALUE;
            break;
        case 0x02:  /* use internal low speed RC */
            SystemCoreClock = LRC_VALUE;
            break;
        case 0x03:  /* use external high speed OSC */
            SystemCoreClock = XTAL_VALUE;
            break;
        case 0x04:  /* use external low speed OSC */
            SystemCoreClock = XTAL32_VALUE;
            break;
        case 0x05:  /* use MPLL */
            /* PLLCLK = ((pllsrc / pllm) * plln) / pllp */
            pllsource = M4_SYSREG->CMU_PLLCFGR_f.PLLSRC;
            plln = M4_SYSREG->CMU_PLLCFGR_f.MPLLN;
            pllp = M4_SYSREG->CMU_PLLCFGR_f.MPLLP;
            pllm = M4_SYSREG->CMU_PLLCFGR_f.MPLLM;
            /* use exteranl high speed OSC as PLL source */
            if (0ul == pllsource)
            {
                SystemCoreClock = (XTAL_VALUE) / (pllm + 1ul) * (plln + 1ul) / (pllp + 1ul);
            }
            /* use interanl high RC as PLL source */
            else if (1ul == pllsource)
            {
                SystemCoreClock = (HRC_VALUE) / (pllm + 1ul) * (plln + 1ul) / (pllp + 1ul);
            }
            else
            {
                /* Reserved */
            }
            break;
    }
}

 
void SysClkConfig(void)
{
    stc_clk_sysclk_cfg_t    stcSysClkCfg;  //系统时钟
    stc_clk_xtal_cfg_t      stcXtalCfg;    //晶振配置
    stc_clk_mpll_cfg_t      stcMpllCfg;    //PLL
    stc_sram_config_t       stcSramConfig;
    
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);
 
    /* Set bus clk div.    分频 */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;  // 100MHz  
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;  // 50MHz
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;  // 100MHz
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;  // 50MHz
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;  // 25MHz
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;  // 25MHz
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;  // 50MHz
    CLK_SysClkConfig(&stcSysClkCfg);//时钟分频
 
    /* Switch system clock source to MPLL. */
    /* Use Xtal as MPLL source. */
    stcXtalCfg.enMode        = ClkXtalModeOsc;//XTAL模式选择位 
    stcXtalCfg.enDrv         = ClkXtalLowDrv;//XTAL驱动能力选择   
    stcXtalCfg.enFastStartup = Enable;//XTAL超高速驱动允许  
    CLK_XtalConfig(&stcXtalCfg);//CMU XTAL  配置寄存器
    CLK_XtalCmd(Enable);//开启CMU XTAL  
    while(Set != CLK_GetFlagStatus(ClkFlagXTALRdy))
    {
        ;
    }
    /* MPLL config. */
    stcMpllCfg.pllmDiv = 1ul;//MPLL输入时钟分频系数
    stcMpllCfg.plln    =50ul;//MPLL倍频系数
    stcMpllCfg.PllpDiv = 4ul;
    stcMpllCfg.PllqDiv = 4ul;
    stcMpllCfg.PllrDiv = 4ul;
    CLK_SetPllSource(ClkPllSrcXTAL);//时钟源选择  XTAL
    CLK_MpllConfig(&stcMpllCfg);//CMU MPLL 时钟分频配置
        
    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(5ul);
    EFM_Lock();
        
    /* sram init include read/write wait cycle setting */
    stcSramConfig.u8SramIdx = Sram12Idx | Sram3Idx | SramHsIdx | SramRetIdx;
    stcSramConfig.enSramRC = SramCycle2;
    stcSramConfig.enSramWC = SramCycle2;
    stcSramConfig.enSramEccMode = EccMode3;
    stcSramConfig.enSramEccOp = SramNmi;
    stcSramConfig.enSramPyOp = SramNmi;
    SRAM_Init(&stcSramConfig);        
 
    /* Enable MPLL. */
    CLK_MpllCmd(Enable);//用于开始停止MPLL。0：MPLL动作开始 1：MPLL停止
 
    /* Wait MPLL ready. */
    while(Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
        ;
    }
    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);//CMU  系统时钟源切换寄存器
}
 
//@} // UsartGroup

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
