/**
 *******************************************************************************
 * @file  mpu/mpu_core_write_protrct/source/core_mpu.c
 * @brief This file provides firmware functions to manage the Core MPU.
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
#include "core_mpu.h"

/**
 * @addtogroup HC32F460_DDL_Examples
 * @{
 */

/**
 * @addtogroup MPU_Core_Write_Protect
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

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
 * @defgroup Core_MPU_Global_Functions Core_MPU Global Functions
 * @{
 */

/**
 * @brief  Core MPU configuration.
 * @param  [in] pstcMpuConfig           Pointer to a @ref stc_core_mpu_config_t structure
 * @retval int32_t:
 *           - LL_OK: Initialize success
 *           - LL_ERR_INVD_PARAM: pstcMpuConfig == NULL
 */
int32_t CORE_MPU_Config(const stc_core_mpu_config_t *pstcMpuConfig)
{
    int32_t i32Ret = LL_OK;

    if (NULL == pstcMpuConfig) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        /* Set the Region number */
        MODIFY_REG32(MPU->CTRL, (MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk),
                     (pstcMpuConfig->u32BackgroundPermission | pstcMpuConfig->u32NmiFaultPermission));
    }

    return i32Ret;
}

/**
 * @brief  Get the Core region number.
 * @param  None
 * @retval uint8_t                      Total number of the region.
 */
uint8_t CORE_MPU_GetRegionNum(void)
{
    return (uint8_t)(READ_REG32_BIT(MPU->TYPE, MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos);
}

/**
 * @brief  Enable or disable the Core MPU function.
 * @param  [in] enNewState              An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void CORE_MPU_Cmd(en_functional_state_t enNewState)
{
    if (DISABLE != enNewState) {
        /* Enable the MPU */
        SET_REG32_BIT(MPU->CTRL, MPU_CTRL_ENABLE_Msk);
        /* Enable fault exception */
        SET_REG32_BIT(SCB->SHCSR, SCB_SHCSR_MEMFAULTENA_Msk);
        /* Ensure MPU setting take effects */
        __DSB();
        __ISB();
    } else {
        __DMB();
        /* Disable fault exception */
        CLR_REG32_BIT(SCB->SHCSR, SCB_SHCSR_MEMFAULTENA_Msk);
        /* Disable the MPU*/
        CLR_REG32_BIT(MPU->CTRL, MPU_CTRL_ENABLE_Msk);
    }
}

/**
 * @brief  Initialize the region.
 * @param  [in] u8Num                   The number of the regsion.
 *         This parameter can be one of the following values:
 *           @arg CORE_MPU_REGION_NUM0: Core MPU region number 0
 *           @arg CORE_MPU_REGION_NUM1: Core MPU region number 1
 *           @arg CORE_MPU_REGION_NUM2: Core MPU region number 2
 *           @arg CORE_MPU_REGION_NUM3: Core MPU region number 3
 *           @arg CORE_MPU_REGION_NUM4: Core MPU region number 4
 *           @arg CORE_MPU_REGION_NUM5: Core MPU region number 5
 *           @arg CORE_MPU_REGION_NUM6: Core MPU region number 6
 *           @arg CORE_MPU_REGION_NUM7: Core MPU region number 7
 * @param  [in] pstcRegionInit          Pointer to a @ref stc_core_mpu_region_init_t structure
 * @retval int32_t:
 *           - LL_OK: Initialize success
 *           - LL_ERR_INVD_PARAM: Invalid parameter
 */
int32_t CORE_MPU_RegionInit(uint8_t u8Num, const stc_core_mpu_region_init_t *pstcRegionInit)
{
    int32_t i32Ret = LL_OK;

    if (NULL == pstcRegionInit) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        /* Set the Region number */
        WRITE_REG32(MPU->RNR, u8Num);
        WRITE_REG32(MPU->RBAR, pstcRegionInit->u32BaseAddr);
        WRITE_REG32(MPU->RASR, (pstcRegionInit->u32Size             | pstcRegionInit->u32InstruAccess     |
                                pstcRegionInit->u32AccessPermission | pstcRegionInit->u32TypeExtend       |
                                pstcRegionInit->u32Bufferable       | pstcRegionInit->u32Shareable        |
                                pstcRegionInit->u32Cacheable        |
                                (pstcRegionInit->u32SubRegion << MPU_RASR_SRD_Pos)));
    }

    return i32Ret;
}

/**
 * @brief  Fills each stc_core_mpu_region_init_t member with default value.
 * @param  [out] pstcRegionInit         Pointer to a @ref stc_core_mpu_region_init_t structure
 * @retval int32_t:
 *           - LL_OK: stc_core_mpu_region_init_t member initialize success
 *           - LL_ERR_INVD_PARAM: Invalid parameter
 */
int32_t CORE_MPU_RegionStructInit(stc_core_mpu_region_init_t *pstcRegionInit)
{
    int32_t i32Ret = LL_OK;

    if (NULL == pstcRegionInit) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        pstcRegionInit->u32BaseAddr         = 0UL;
        pstcRegionInit->u32Size             = CORE_MPU_REGION_SIZE_32BYTE;
        pstcRegionInit->u32InstruAccess     = CORE_MPU_INSTRU_ACCESS_ENABLE;
        pstcRegionInit->u32AccessPermission = CORE_MPU_REGION_FULL_ACCESS;
        pstcRegionInit->u32TypeExtend       = CORE_MPU_TYPE_EXTEND_LVL0;
        pstcRegionInit->u32SubRegion        = 0UL;
        pstcRegionInit->u32Shareable        = CORE_MPU_ACCESS_NOT_SHAREABLE;
        pstcRegionInit->u32Cacheable        = CORE_MPU_ACCESS_NOT_CACHEABLE;
        pstcRegionInit->u32Bufferable       = CORE_MPU_ACCESS_NOT_BUFFERABLE;
    }

    return i32Ret;
}

/**
 * @brief  Enable or disable the MPU function of the region.
 * @param  [in] u8Num                   The number of the regsion.
 *         This parameter can be one of the following values:
 *           @arg CORE_MPU_REGION_NUM0: Core MPU region number 0
 *           @arg CORE_MPU_REGION_NUM1: Core MPU region number 1
 *           @arg CORE_MPU_REGION_NUM2: Core MPU region number 2
 *           @arg CORE_MPU_REGION_NUM3: Core MPU region number 3
 *           @arg CORE_MPU_REGION_NUM4: Core MPU region number 4
 *           @arg CORE_MPU_REGION_NUM5: Core MPU region number 5
 *           @arg CORE_MPU_REGION_NUM6: Core MPU region number 6
 *           @arg CORE_MPU_REGION_NUM7: Core MPU region number 7
 * @param  [in] enNewState              An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void CORE_MPU_RegionCmd(uint8_t u8Num, en_functional_state_t enNewState)
{
    /* Set the Region number */
    WRITE_REG32(MPU->RNR, u8Num);
    if (DISABLE != enNewState) {
        SET_REG32_BIT(MPU->RASR, MPU_RASR_ENABLE_Msk);
    } else {
        CLR_REG32_BIT(MPU->RASR, MPU_RASR_ENABLE_Msk);
    }
}

/**
 * @}
 */

/**
 * @}
 */

/**
* @}
*/

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
