/**
 *******************************************************************************
 * @file  functional_safety/iec60730_class_b/source/stl_bsp_conf.h
 * @brief This file contains STL BSP resource configure.
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

#ifndef __STL_BSP_CONF_H__
#define __STL_BSP_CONF_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup STL_IEC60730_Application
 * @{
 */

/**
 * @addtogroup IEC60730_STL_BSP_Configure
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
*******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/**
 * @defgroup STL_Function_Configure STL Function Configure
 * @brief This is the list of function to be used in the STL Library.
 * Select the functions you need to use to STL_ON.
 * @{
 */
#define STL_PRINT_ENABLE                STL_ON
#define STL_RESET_AT_FAILURE            STL_OFF
/**
 * @}
 */

/**
 * @defgroup STL_Print_Configure STL Print Configure
 * @{
 */
#if (STL_PRINT_ENABLE == STL_ON)

#define STL_PRINTF_DEVICE               (CM_USART4)
#define STL_PRINTF_DEVICE_FCG_ENALBE()  FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART4, ENABLE)

#define STL_PRINTF_BAUDRATE             (115200UL)
#define STL_PRINTF_BAUDRATE_ERR_MAX     (0.025F)

#define STL_PRINTF_PORT                 (GPIO_PORT_E)
#define STL_PRINTF_PIN                  (GPIO_PIN_06)
#define STL_PRINTF_PORT_FUNC            (GPIO_FUNC_36)

#endif
/**
 * @}
 */

/**
 * @defgroup STL_RAM_Configure STL RAM Configure
 * @{
 */
#define STL_RAM1_START                  (0x1FFF8000UL)
#define STL_RAM1_END                    (0x20026FFFUL)

#define STL_RAM2_START                  (0x200F0000UL)
#define STL_RAM2_END                    (0x200F0FFFUL)

/* Memory address must be coherent with MARCH_RAM in linker *.sct */
#define STL_MARCH_RAM_START             (0x1FFF8000UL)
#define STL_MARCH_RAM_END               (0x1FFF801FUL)
/**
 * @}
 */

/**
 * @defgroup STL_ROM_Configure STL ROM Configure
 * @{
 */
#define STL_ROM_START                   (0x00000000UL)
#define STL_ROM_END                     (0x0007FFFFUL)
#define STL_ROM_SIZE                    (ROM_END - ROM_START + 1UL)
/**
 * @}
 */

/**
 * @defgroup STL_Systick_Configure STL Systick Configure
 * @{
 */
#define STL_SYSTICK_TICK_FREQ           (100UL)                                     /* Frequency: 100Hz */
#define STL_SYSTICK_TICK_VALUE          (SystemCoreClock / STL_SYSTICK_TICK_FREQ)   /* Period value: 10ms */
/**
 * @}
 */

/**
 * @defgroup STL_Interrupt_Resource_Configure STL Interrupt Resource Configure definition
 * @{
 */

/* Clock test */
#define STL_FCM_ERR_INT_IRQn            (INT000_IRQn)
#define STL_FCM_ERR_IRQ_PRIO            (DDL_IRQ_PRIO_DEFAULT)

#define STL_FCM_OVF_INT_IRQn            (INT001_IRQn)
#define STL_FCM_OVF_IRQ_PRIO            (DDL_IRQ_PRIO_DEFAULT)

/* Interrupt test (four timer-x,y,z,w) */
#define STL_TMRx_OVF_INT_SRC            (INT_SRC_TMRA_1_OVF)
#define STL_TMRx_OVF_INT_IRQn           (INT010_IRQn)
#define STL_TMRx_OVF_IRQ_PRIO           (DDL_IRQ_PRIO_05)

#define STL_TMRy_OVF_INT_SRC            (INT_SRC_TMRA_2_OVF)
#define STL_TMRy_OVF_INT_IRQn           (INT011_IRQn)
#define STL_TMRy_OVF_IRQ_PRIO           (DDL_IRQ_PRIO_04)

#define STL_TMRz_OVF_INT_SRC            (INT_SRC_TMRA_3_OVF)
#define STL_TMRz_OVF_INT_IRQn           (INT012_IRQn)
#define STL_TMRz_OVF_IRQ_PRIO           (DDL_IRQ_PRIO_03)

#define STL_TMRw_OVF_INT_SRC            (INT_SRC_TMRA_4_OVF)
#define STL_TMRw_OVF_INT_IRQn           (INT013_IRQn)
#define STL_TMRw_OVF_IRQ_PRIO           (DDL_IRQ_PRIO_02)

/* ADC test */
#define STL_ADC_AWD_INT_SRC             (INT_SRC_ADC1_CHCMP)
#define STL_ADC_AWD_INT_IRQn            (INT016_IRQn)
#define STL_ADC_AWD_IRQ_PRIO            (DDL_IRQ_PRIO_DEFAULT)
/**
 * @}
 */

/**
 * @defgroup STL_ADC_Test_Resource_Configure STL ADC Test Resource Configure
 * @{
 */
#define STL_ADC_FCG_ENABLE()            FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC1, ENABLE)

#define STL_ADC_UNIT                    (CM_ADC1)
#define STL_ADC_CH                      (ADC_CH10)

#define STL_ADC_PORT                    (GPIO_PORT_C)
#define STL_ADC_PIN                     (GPIO_PIN_00)

#define STL_ADC_VREF                    (3.3F)
#define STL_ADC_AWD_LOW_THRESHOLD_VOL   (1.6F)
#define STL_ADC_AWD_HIGH_THRESHOLD_VOL  (1.7F)
#define STL_ADC_ACCURACY                (1UL << 12U)

#define STL_ADC_AWD_UNIT                (ADC_AWD0)
#define STL_ADC_AWD_FLAG                (ADC_AWD_FLAG_CH10)
#define STL_ADC_AWD_TYPE                (ADC_AWD_INT_SEQA)
#define STL_ADC_CAL_VOL(adcVal)         ((float32_t)(adcVal) * STL_ADC_VREF) / ((float32_t)STL_ADC_ACCURACY)
#define STL_ADC_CAL_VAL(vol)            ((uint16_t)(((float32_t)(vol) * (float32_t)STL_ADC_ACCURACY) / (float32_t)STL_ADC_VREF))
#define STL_ADC_WINDOW_LOW              (STL_ADC_CAL_VAL(STL_ADC_AWD_LOW_THRESHOLD_VOL) + 8U)
#define STL_ADC_WINDOW_HIGH             (STL_ADC_CAL_VAL(STL_ADC_AWD_HIGH_THRESHOLD_VOL) - 8U)
/**
 * @}
 */

/**
 * @defgroup STL_Interrupt_Test_Resource_Configure STL Interrupt Test Resource Configure
 * @{
 */
#define STL_TMR_FCG                     (FCG2_PERIPH_TMRA_1 | FCG2_PERIPH_TMRA_2 | \
                                         FCG2_PERIPH_TMRA_3 | FCG2_PERIPH_TMRA_4)
#define STL_TMR_FCG_ENABLE()            FCG_Fcg2PeriphClockCmd(STL_TMR_FCG, ENABLE)

#define STL_TMRx_UNIT                   (CM_TMRA_1)
#define STL_TMRx_INT_OVF                (TMRA_INT_OVF)
#define STL_TMRx_FLAG_OVF               (TMRA_FLAG_OVF)
#define STL_TMRx_FREQ                   (STL_SYSTICK_TICK_FREQ / 4UL)
#define STL_TMRx_FREQ_OFFSET            (5UL)

#define STL_TMRy_UNIT                   (CM_TMRA_2)
#define STL_TMRy_INT_OVF                (TMRA_INT_OVF)
#define STL_TMRy_FLAG_OVF               (TMRA_FLAG_OVF)
#define STL_TMRy_FREQ                   (STL_SYSTICK_TICK_FREQ / 2UL)
#define STL_TMRy_FREQ_OFFSET            (5UL)

#define STL_TMRz_UNIT                   (CM_TMRA_3)
#define STL_TMRz_INT_OVF                (TMRA_INT_OVF)
#define STL_TMRz_FLAG_OVF               (TMRA_FLAG_OVF)
#define STL_TMRz_FREQ                   (STL_SYSTICK_TICK_FREQ + STL_SYSTICK_TICK_FREQ / 4UL)
#define STL_TMRz_FREQ_OFFSET            (5UL)

#define STL_TMRw_UNIT                   (CM_TMRA_4)
#define STL_TMRw_INT_OVF                (TMRA_INT_OVF)
#define STL_TMRw_FLAG_OVF               (TMRA_FLAG_OVF)
#define STL_TMRw_FREQ                   (STL_SYSTICK_TICK_FREQ + STL_SYSTICK_TICK_FREQ / 2UL)
#define STL_TMRw_FREQ_OFFSET            (5UL)
/**
 * @}
 */

/**
 * @defgroup STL_IO_Test_Resource_Configure STL IO Test Resource Configure
 * @{
 */
#define STL_INPUT_PORTx                 (GPIO_PORT_B)
#define STL_INPUT_PORTx_ALL_PINS        (GPIO_PIN_B_ALL)
#define STL_INPUT_PORTx_TEST_PINS       (GPIO_PIN_01)
#define STL_INPUT_PORTx_EXPECT_VAL      (STL_INPUT_PORTx_TEST_PINS)

#define STL_OUTPUT_PORTx                (GPIO_PORT_D)
#define STL_OUTPUT_PORTx_ALL_PINS       (GPIO_PIN_E_ALL)
#define STL_OUTPUT_PORTx_TEST_PINS      (GPIO_PIN_03 | GPIO_PIN_04 | GPIO_PIN_05 | GPIO_PIN_06)
#define STL_OUTPUT_PORTx_OUT_VAL        (STL_OUTPUT_PORTx_TEST_PINS)
#define STL_OUTPUT_PORTx_EXPECT_VAL     (STL_OUTPUT_PORTx_TEST_PINS)
/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __STL_BSP_CONF_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
