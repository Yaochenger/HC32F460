/**
 *******************************************************************************
 * @file  aes/aes_base/source/main.c
 * @brief Main program of AES for the Device Driver Library.
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
 * @addtogroup AES_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* AES key byte size. */
#define AES_KEY_SIZE                        (16U)

/* Size of plaintext and ciphertext. */
#define AES_PLAINTEXT_BYTE_SIZE             (16UL * 1UL)
#define AES_CIPHERTEXT_BYTE_SIZE            (AES_PLAINTEXT_BYTE_SIZE)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void AesConfig(void);
static int32_t AesVerify(const uint8_t *pu8Data1, const uint8_t *pu8Data2, uint32_t u32NumByte);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
const static char *m_strPlaintext = "abcdefghijklmnop";

const static char *m_strAesKey = "1234567890abcdef";

const static uint8_t m_au8Ciphertext[AES_CIPHERTEXT_BYTE_SIZE] = {
    0x2E, 0xE0, 0xF9, 0x5A, 0x84, 0x51, 0x70, 0x7A,
    0xB5, 0xB6, 0xE1, 0x16, 0x65, 0x01, 0xCB, 0x1F,
};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of example project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t au8Plaintext[AES_PLAINTEXT_BYTE_SIZE];
    uint8_t au8Ciphertext[AES_CIPHERTEXT_BYTE_SIZE];

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* AES configuration. */
    AesConfig();
    /* Initializes UART for debug printing. Baudrate is 19200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 19200U, BSP_PRINTF_Preinit);
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    for (;;) {
        /* AES encryption. */
        (void)AES_Encrypt((uint8_t *)m_strPlaintext, AES_PLAINTEXT_BYTE_SIZE, \
                          (uint8_t *)m_strAesKey, AES_KEY_SIZE, \
                          au8Ciphertext);
        if (AesVerify(au8Ciphertext, m_au8Ciphertext, AES_CIPHERTEXT_BYTE_SIZE) != LL_OK) {
            DDL_Printf("AES encryption FAIL.\r\n");
            for (;;) {
                /* rsvd */
            }
        }
        DDL_Printf("AES encryption OK.\r\n");

        /* AES decryption */
        (void)AES_Decrypt(m_au8Ciphertext, AES_CIPHERTEXT_BYTE_SIZE, \
                          (uint8_t *)m_strAesKey, AES_KEY_SIZE, \
                          au8Plaintext);
        if (AesVerify(au8Plaintext, (uint8_t *)m_strPlaintext, AES_PLAINTEXT_BYTE_SIZE) != LL_OK) {
            DDL_Printf("AES decryption FAIL.\r\n");
            for (;;) {
                /* rsvd */
            }
        }
        DDL_Printf("AES decryption OK.\r\n");

        DDL_DelayMS(500U);
    }
}

/**
 * @brief  AES configuration.
 * @param  None
 * @retval None
 */
static void AesConfig(void)
{
    /* Enable AES peripheral clock. */
    FCG_Fcg0PeriphClockCmd(PWC_FCG0_AES, ENABLE);
}

/**
 * @brief  AES verification.
 * @param  [in]  pu8Data1               Pointer to data 1.
 * @param  [in]  pu8Data2               Pointer to data 2.
 * @param  [in]  u32NumByte             Number of byte to verify.
 * @retval int32_t:
 *         - LL_OK:                     Verification OK.
 *         - LL_ERR:                    Verification FAIL.
 */
static int32_t AesVerify(const uint8_t *pu8Data1, const uint8_t *pu8Data2, uint32_t u32NumByte)
{
    uint32_t i;
    int32_t i32Ret = LL_OK;

    for (i = 0UL; i < u32NumByte; i++) {
        if (pu8Data1[i] != pu8Data2[i]) {
            i32Ret = LL_ERR;
            break;
        }
    }

    return i32Ret;
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
