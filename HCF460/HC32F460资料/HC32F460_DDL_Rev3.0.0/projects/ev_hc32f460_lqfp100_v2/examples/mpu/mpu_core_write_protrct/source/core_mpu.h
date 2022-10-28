/**
 *******************************************************************************
 * @file  mpu/mpu_core_write_protrct/source/core_mpu.h
 * @brief This file contains all the functions prototypes of the Core MPU driver
 *        library.
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
#ifndef __CORE_MPU_H__
#define __CORE_MPU_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_def.h"

#include "hc32f4xx.h"

/**
 * @addtogroup HC32F460_DDL_Examples
 * @{
 */

/**
 * @addtogroup MPU_Core_Write_Protect
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 * @defgroup Core_MPU_Global_Types Core_MPU Global Types
 * @{
 */

/**
 * @brief Core MPU configuration structure definition
 */
typedef struct {
    uint32_t u32BackgroundPermission;   /*!< Specifies the background region access permission in privilege mode.
                                             This parameter can be a value of @ref Core_MPU_Background_Access_Permission  */
    uint32_t u32NmiFaultPermission;     /*!< Specifies the validity of the MPU function during the HardFault handler and NMI handler.
                                             This parameter can be a value of @ref Core_MPU_Exception_Access_Permission   */
} stc_core_mpu_config_t;

/**
 * @brief Core MPU Region initialization structure definition
 */
typedef struct {
    uint32_t u32BaseAddr;               /*!< Specifies the base address of the region.
                                             This parameter can be a number between 0UL and 0xFFFFFFE0UL             */
    uint32_t u32Size;                   /*!< Specifies the size of the region.
                                             This parameter can be a value of @ref Core_MPU_Region_Size              */
    uint32_t u32InstruAccess;           /*!< Specifies the validity of the instruction access.
                                             This parameter can be a value of @ref Core_MPU_Instruction_Access       */
    uint32_t u32AccessPermission;       /*!< Specifies the region access permission.
                                             This parameter can be a value of @ref Core_MPU_Region_Access_Permission */
    uint32_t u32TypeExtend;             /*!< Specifies the type extension level.
                                             This parameter can be a value of @ref Core_MPU_Type_Extend              */
    uint32_t u32SubRegion;              /*!< Specifies the number of the subregion protection to disable.
                                             This parameter can be a number between 0UL and 0xFFU                    */
    uint32_t u32Shareable;              /*!< Specifies the shareable status of the protected region.
                                             This parameter can be a value of @ref Core_MPU_Access_Shareable         */
    uint32_t u32Cacheable;              /*!< Specifies the cacheable status of the region protected.
                                             This parameter can be a value of @ref Core_MPU_Access_Cacheable         */
    uint32_t u32Bufferable;             /*!< Specifies the bufferable status of the protected region.
                                             This parameter can be a value of @ref Core_MPU_Access_Bufferable        */
} stc_core_mpu_region_init_t;

/**
 * @}
 */

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup Core_MPU_Global_Macros Core_MPU Global Macros
 * @{
 */

/**
 * @defgroup Core_MPU_Background_Access_Permission Core MPU Background Access Permission
 * @{
 */
#define CORE_MPU_BKGRD_PRIV_ACCESS_DISABLE          (0UL)
#define CORE_MPU_BKGRD_PRIV_ACCESS_ENABLE           (MPU_CTRL_PRIVDEFENA_Msk)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Exception_Access_Permission Core MPU Exception Access Permission
 * @{
 */
#define CORE_MPU_NMI_FAULT_DISABLE_MPU              (0UL)
#define CORE_MPU_NMI_FAULT_ENABLE_MPU               (MPU_CTRL_HFNMIENA_Msk)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Region_Size Core MPU Region Size
 * @{
 */
#define CORE_MPU_REGION_SIZE_32BYTE                 (0x04UL)
#define CORE_MPU_REGION_SIZE_64BYTE                 (0x05UL)
#define CORE_MPU_REGION_SIZE_128BYTE                (0x06UL)
#define CORE_MPU_REGION_SIZE_256BYTE                (0x07UL)
#define CORE_MPU_REGION_SIZE_512BYTE                (0x08UL)
#define CORE_MPU_REGION_SIZE_1KBYTE                 (0x09UL)
#define CORE_MPU_REGION_SIZE_2KBYTE                 (0x0AUL)
#define CORE_MPU_REGION_SIZE_4KBYTE                 (0x0BUL)
#define CORE_MPU_REGION_SIZE_8KBYTE                 (0x0CUL)
#define CORE_MPU_REGION_SIZE_16KBYTE                (0x0DUL)
#define CORE_MPU_REGION_SIZE_32KBYTE                (0x0EUL)
#define CORE_MPU_REGION_SIZE_64KBYTE                (0x0FUL)
#define CORE_MPU_REGION_SIZE_128KBYTE               (0x10UL)
#define CORE_MPU_REGION_SIZE_256KBYTE               (0x11UL)
#define CORE_MPU_REGION_SIZE_512KBYTE               (0x12UL)
#define CORE_MPU_REGION_SIZE_1MBYTE                 (0x13UL)
#define CORE_MPU_REGION_SIZE_2MBYTE                 (0x14UL)
#define CORE_MPU_REGION_SIZE_4MBYTE                 (0x15UL)
#define CORE_MPU_REGION_SIZE_8MBYTE                 (0x16UL)
#define CORE_MPU_REGION_SIZE_16MBYTE                (0x17UL)
#define CORE_MPU_REGION_SIZE_32MBYTE                (0x18UL)
#define CORE_MPU_REGION_SIZE_64MBYTE                (0x19UL)
#define CORE_MPU_REGION_SIZE_128MBYTE               (0x1AUL)
#define CORE_MPU_REGION_SIZE_256MBYTE               (0x1BUL)
#define CORE_MPU_REGION_SIZE_512MBYTE               (0x1CUL)
#define CORE_MPU_REGION_SIZE_1GBYTE                 (0x1DUL)
#define CORE_MPU_REGION_SIZE_2GBYTE                 (0x1EUL)
#define CORE_MPU_REGION_SIZE_4GBYTE                 (0x1FUL)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Instruction_Access Core MPU Instruction Access
 * @{
 */
#define CORE_MPU_INSTRU_ACCESS_DISABLE              (MPU_RASR_XN_Msk)
#define CORE_MPU_INSTRU_ACCESS_ENABLE               (0UL)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Region_Access_Permission Core MPU Region Access Permission
 * @{
 */
#define CORE_MPU_REGION_NO_ACCESS                   (0UL)
#define CORE_MPU_REGION_PRIV_RW                     (0x01UL << MPU_RASR_AP_Pos)
#define CORE_MPU_REGION_PRIV_RW_USER_RO             (0x02UL << MPU_RASR_AP_Pos)
#define CORE_MPU_REGION_FULL_ACCESS                 (0x03UL << MPU_RASR_AP_Pos)
#define CORE_MPU_REGION_PRIV_RO                     (0x05UL << MPU_RASR_AP_Pos)
#define CORE_MPU_REGION_PRIV_RO_USER_RO             (0x06UL << MPU_RASR_AP_Pos)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Type_Extend Core MPU Type Extend
 * @{
 */
#define CORE_MPU_TYPE_EXTEND_LVL0                   (0UL)
#define CORE_MPU_TYPE_EXTEND_LVL1                   (0x01UL << MPU_RASR_TEX_Pos)
#define CORE_MPU_TYPE_EXTEND_LVL2                   (0x02UL << MPU_RASR_TEX_Pos)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Access_Shareable Core MPU Instruction Access Shareable
 * @{
 */
#define CORE_MPU_ACCESS_NOT_SHAREABLE               (0UL)
#define CORE_MPU_ACCESS_SHAREABLE                   (MPU_RASR_S_Msk)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Access_Cacheable Core MPU Instruction Access Cacheable
 * @{
 */
#define CORE_MPU_ACCESS_NOT_CACHEABLE               (0UL)
#define CORE_MPU_ACCESS_CACHEABLE                   (MPU_RASR_C_Msk)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Access_Bufferable Core MPU Instruction Access Bufferable
 * @{
 */
#define CORE_MPU_ACCESS_NOT_BUFFERABLE              (0UL)
#define CORE_MPU_ACCESS_BUFFERABLE                  (MPU_RASR_B_Msk)
/**
 * @}
 */

/**
 * @defgroup Core_MPU_Region_Number CORTEX MPU Region Number
 * @{
 */
#define CORE_MPU_REGION_NUM0                        (0U)
#define CORE_MPU_REGION_NUM1                        (0x01U)
#define CORE_MPU_REGION_NUM2                        (0x02U)
#define CORE_MPU_REGION_NUM3                        (0x03U)
#define CORE_MPU_REGION_NUM4                        (0x04U)
#define CORE_MPU_REGION_NUM5                        (0x05U)
#define CORE_MPU_REGION_NUM6                        (0x06U)
#define CORE_MPU_REGION_NUM7                        (0x07U)
/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup Core_MPU_Global_Functions
 * @{
 */

int32_t CORE_MPU_Config(const stc_core_mpu_config_t *pstcMpuConfig);
uint8_t CORE_MPU_GetRegionNum(void);
void CORE_MPU_Cmd(en_functional_state_t enNewState);

int32_t CORE_MPU_RegionInit(uint8_t u8Num, const stc_core_mpu_region_init_t *pstcRegionInit);
int32_t CORE_MPU_RegionStructInit(stc_core_mpu_region_init_t *pstcRegionInit);
void CORE_MPU_RegionCmd(uint8_t u8Num, en_functional_state_t enNewState);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CORE_MPU_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
