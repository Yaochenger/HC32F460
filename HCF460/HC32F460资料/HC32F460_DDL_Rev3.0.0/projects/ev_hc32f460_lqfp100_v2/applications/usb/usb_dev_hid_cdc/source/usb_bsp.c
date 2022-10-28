/**
 *******************************************************************************
 * @file  usb/usb_dev_hid_cdc/source/usb_bsp.c
 * @brief BSP function for USB example
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
#include "usb_bsp.h"
#include "hc32_ll.h"
#include "usb_dev_custom_hid_class.h"
#include "usb_dev_driver.h"
#include "usb_dev_int.h"

/**
 * @addtogroup HC32F460_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Hid_Cdc
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
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

/* KEY */
#define KEY_PORT                (GPIO_PORT_B)
#define KEY_PIN                 (GPIO_PIN_01)
#define KEY_EXTINT_CH           (EXTINT_CH01)
#define KEY_INT_SRC             (INT_SRC_PORT_EIRQ1)
#define KEY_IRQn                (INT003_IRQn)

/* USBFS Core*/
#define USB_DP_PORT             (GPIO_PORT_A)
#define USB_DP_PIN              (GPIO_PIN_12)
#define USB_DM_PORT             (GPIO_PORT_A)
#define USB_DM_PIN              (GPIO_PIN_11)
#define USB_VBUS_PORT           (GPIO_PORT_A)
#define USB_VBUS_PIN            (GPIO_PIN_09)
#define USB_SOF_PORT            (GPIO_PORT_A)
#define USB_SOF_PIN             (GPIO_PIN_08)

#define TMR0x                   (CM_TMR0_1)
#define TMR0_CLK                (FCG2_PERIPH_TMR0_1)
#define TMR0_CH_x               (TMR0_CH_A)
#define TMR0_INT_TYPE           (TMR0_INT_CMP_A)
#define TMR0_FLAG               (TMR0_FLAG_CMP_A)
/* TMR0 interrupt source and number define */
#define TMR0_IRQn               (INT014_IRQn)
#define TMR0_SOURCE             (INT_SRC_TMR0_1_CMP_A)

#define TMR0_CLK_SRC            (TMR0_CLK_SRC_LRC)
#define TMR0_CLK_DIV            (TMR0_CLK_DIV16)
#define TMR0_CMP_VAL            (32768UL/16UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
uint8_t PrevXferDone = 1U;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
extern  usb_core_instance usb_dev;
extern  void usb_isr_handler(usb_core_instance *pdev);

/**
 * @brief  handle the USB interrupt
 * @param  None
 * @retval None
 */
static void USB_IRQ_Handler(void)
{
    usb_isr_handler(&usb_dev);
}

/**
 * @brief  KEY External interrupt Ch.0 callback function
 * @param  None
 * @retval None
 */
static void EXINT_IrqCallback(void)
{
    if (SET == EXTINT_GetExtIntStatus(KEY_EXTINT_CH)) {
        if ((0U != PrevXferDone) && (usb_dev.dev.device_cur_status == USB_DEV_CONFIGURED)) {
            Send_Buf[0U] = KEY_REPORT_ID;
            if (PIN_RESET == GPIO_ReadInputPins(KEY_PORT, KEY_PIN)) {
                Send_Buf[1] = 0x01U;
            } else {
                Send_Buf[1] = 0x00U;
            }
            usb_deveptx(&usb_dev, HID_IN_EP, Send_Buf, 2);
            PrevXferDone = 0U;
        }
        EXTINT_ClearExtIntStatus(KEY_EXTINT_CH);
    }
}

/**
 * @brief  configure the gpio related with the KEY and the NVIC
 * @param  None
 * @retval None
 */
static void KEY_Init(void)
{
    stc_extint_init_t stcExintInit;
    stc_irq_signin_config_t stcIrqSignConfig;
    stc_gpio_init_t stcGpioInit;

    /* GPIO config */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16ExtInt = PIN_EXTINT_ON;
    stcGpioInit.u16PullUp = PIN_PU_ON;
    (void)GPIO_Init(KEY_PORT, KEY_PIN, &stcGpioInit);

    /* Exint config */
    (void)EXTINT_StructInit(&stcExintInit);
    stcExintInit.u32Edge = EXTINT_TRIG_FALLING;
    (void)EXTINT_Init(KEY_EXTINT_CH, &stcExintInit);

    /* IRQ sign-in */
    stcIrqSignConfig.enIntSrc = KEY_INT_SRC;
    stcIrqSignConfig.enIRQn   = KEY_IRQn;
    stcIrqSignConfig.pfnCallback = &EXINT_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    /* NVIC config */
    NVIC_ClearPendingIRQ(KEY_IRQn);
    NVIC_SetPriority(KEY_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(KEY_IRQn);
}

/**
 * @brief  TMR0_1 channelA compare IRQ callback
 * @param  None
 * @retval None
 */
static void TMR0_1_ChACmp_IrqCallback(void)
{
    if ((0U != PrevXferDone) && (usb_dev.dev.device_cur_status == USB_DEV_CONFIGURED)) {
        usb_deveptx(&usb_dev, HID_IN_EP, NULL, 0);
        PrevXferDone = 0U;
    }
    /* Clear the compare matching flag */
    TMR0_ClearStatus(TMR0x, TMR0_FLAG);
}

/**
 * @brief  initialize the Timer0
 * @param  None
 * @retval None
 */
static void TMR0_Init_1S(void)
{
    stc_tmr0_init_t stcTmr0Init;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Enable timer0 peripheral clock */
    FCG_Fcg2PeriphClockCmd(TMR0_CLK, ENABLE);

    /* TIMER0 basetimer function initialize */
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV;        /* Config clock division */
    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC;          /* Chose clock source */
    stcTmr0Init.u32Func = TMR0_FUNC_CMP;            /* Timer0 compare mode */
    stcTmr0Init.u16CompareValue = (uint16_t)TMR0_CMP_VAL;             /* Set compara register data */
    (void)TMR0_Init(TMR0x, TMR0_CH_x, &stcTmr0Init);
    /* In asynchronous clock, If you want to write a TMR0 register, you need to wait for at
       least 3 asynchronous clock cycles after the last write operation! */
    DDL_DelayMS(1U); /* Wait at least 3 asynchronous clock cycles.*/
    /* Timer0 interrupt function Enable */
    TMR0_IntCmd(TMR0x, TMR0_INT_TYPE, ENABLE);
    DDL_DelayMS(1U); /* Wait at least 3 asynchronous clock cycles.*/

    /* Register IRQ handler && configure NVIC. */
    stcIrqSignConfig.enIRQn = TMR0_IRQn;
    stcIrqSignConfig.enIntSrc = TMR0_SOURCE;
    stcIrqSignConfig.pfnCallback = &TMR0_1_ChACmp_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    /* Timer0 ch1 start counting */
    TMR0_Start(TMR0x, TMR0_CH_x);
}

/**
 * @brief  USB clock initial
 * @param  None
 * @retval None
 */
static void UsbClockIni(void)
{
    stc_clock_pllx_init_t stcUpllInit;

    (void)CLK_PLLxStructInit(&stcUpllInit);
    stcUpllInit.u8PLLState = CLK_PLLX_ON;
    stcUpllInit.PLLCFGR = 0UL;
    stcUpllInit.PLLCFGR_f.PLLM = (2UL  - 1UL);
    stcUpllInit.PLLCFGR_f.PLLN = (84UL - 1UL);
    stcUpllInit.PLLCFGR_f.PLLR = (7UL  - 1UL);
    stcUpllInit.PLLCFGR_f.PLLQ = (7UL - 1UL);
    stcUpllInit.PLLCFGR_f.PLLP = (7UL - 1UL); //48M
    (void)CLK_PLLxInit(&stcUpllInit);

    /* Set USB clock source */
    CLK_SetUSBClockSrc(CLK_USBCLK_PLLXP);
}

/**
 * @brief  initialize configurations for the BSP
 * @param  [in] pdev         device instance
 * @retval None
 */
void usb_bsp_init(usb_core_instance *pdev)
{
    stc_gpio_init_t stcGpioCfg;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);

    BSP_CLK_Init();
    BSP_LED_Init();

#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
#endif
    /* USB clock source configurate */
    UsbClockIni();
    /* KEY interrupt function initialize */
    KEY_Init();
    /* Initlialize for 1S interrupt */
    TMR0_Init_1S();

#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("USBFS start !!\r\n");
#endif
    (void)GPIO_StructInit(&stcGpioCfg);
    stcGpioCfg.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(USB_DM_PORT, USB_DM_PIN, &stcGpioCfg);
    (void)GPIO_Init(USB_DP_PORT, USB_DP_PIN, &stcGpioCfg);
    GPIO_SetFunc(USB_VBUS_PORT, USB_VBUS_PIN, GPIO_FUNC_10); /* VBUS */
    FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USBFS, ENABLE);
}

/**
 * @brief  configure the NVIC of USB
 * @param  None
 * @retval None
 */
void usb_bsp_nvicconfig(void)
{
    stc_irq_signin_config_t stcIrqRegiConf;
    /* Register INT_SRC_USBFS_GLB Int to Vect.No.030 */
    stcIrqRegiConf.enIRQn = INT030_IRQn;
    /* Select interrupt function */
    stcIrqRegiConf.enIntSrc = INT_SRC_USBFS_GLB;
    /* Callback function */
    stcIrqRegiConf.pfnCallback = &USB_IRQ_Handler;
    /* Registration IRQ */
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    /* Clear Pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIO_15);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}

/**
 * @brief  This function provides delay time in micro sec
 * @param  [in] usec         Value of delay required in micro sec
 * @retval None
 */
void usb_udelay(const uint32_t usec)
{
    __IO uint32_t i;
    uint32_t j;
    j = (HCLK_VALUE + 1000000UL - 1UL) / 1000000UL * usec;
    for (i = 0UL; i < j; i++) {
    }
}

/**
 * @brief  This function provides delay time in milli sec
 * @param  [in] msec         Value of delay required in milli sec
 * @retval None
 */
void usb_mdelay(const uint32_t msec)
{
    usb_udelay(msec * 1000UL);
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
