/******************************************************************************
* File Name:   main.c
*
* Description: This code example demonstrates the use of GPIO configured as an
*              input pin to generate interrupts in PSoC 4.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2021, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/******************************************************************************
 * Header file includes
 *****************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"


/******************************************************************************
 * Macros
 *****************************************************************************/
#define DELAY_SHORT             (250)   /* milliseconds */
#define DELAY_LONG              (500)   /* milliseconds */

#define LED_BLINK_COUNT         (4)

#define SWITCH_INTR_PRIORITY    (3u)


/*******************************************************************************
* Function Prototypes
********************************************************************************/
void Switch_ISR();


/******************************************************************************
 * Global variable
 *****************************************************************************/
uint32_t interrupt_flag = false;


/******************************************************************************
 * Switch interrupt configuration structure
 *****************************************************************************/
const cy_stc_sysint_t switch_intr_config = {
    .intrSrc = CYBSP_USER_BTN_IRQ,                  /* Source of interrupt signal */
    .intrPriority = SWITCH_INTR_PRIORITY            /* Interrupt priority */
};


/*******************************************************************************
* Function Name: Switch_ISR
********************************************************************************
*
* Summary:
*  This function is executed when GPIO interrupt is triggered.
*
*******************************************************************************/
void Switch_ISR()
{
    /* Clears the triggered pin interrupt */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_NUM);
    NVIC_ClearPendingIRQ(switch_intr_config.intrSrc);

    /* Set interrupt flag */
    interrupt_flag = true;
}


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  System entrance point. This function configures and initializes the GPIO
*  interrupt, update the delay on every GPIO interrupt, and blinks the LED.
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    uint32_t count = 0;
    uint32_t delayMs = DELAY_LONG;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize and enable GPIO interrupt */
    result = Cy_SysInt_Init(&switch_intr_config, Switch_ISR);
    if(result != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Clearing and enabling the GPIO interrupt in NVIC */
    NVIC_ClearPendingIRQ(switch_intr_config.intrSrc);
    NVIC_EnableIRQ(switch_intr_config.intrSrc);

    cy_stc_sysclk_context_t sysClkContext;

    cy_stc_syspm_callback_params_t sysClkCallbackParams =
    {
        .base       = NULL,
        .context    = (void*)&sysClkContext
    };

    /* Callback declaration for Deep Sleep mode */
    cy_stc_syspm_callback_t sysClkCallback =
    {
        .callback       = &Cy_SysClk_DeepSleepCallback,
        .type           = CY_SYSPM_DEEPSLEEP,
        .skipMode       = 0UL,
        .callbackParams = &sysClkCallbackParams,
        .prevItm        = NULL,
        .nextItm        = NULL,
        .order          = 0
    };

    /* Register Deep Sleep callback */
    Cy_SysPm_RegisterCallback(&sysClkCallback);

    for (;;)
    {
        /* Check the interrupt status */
        if(true == interrupt_flag)
        {
            interrupt_flag = false;

            if(DELAY_LONG == delayMs)
            {
                delayMs = DELAY_SHORT;
            }
            else
            {
                delayMs = DELAY_LONG;
            }
        }

        /* Blink LED four times */
        for (count = 0; count < LED_BLINK_COUNT; count++)
        {
            Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_NUM);
            Cy_SysLib_Delay(delayMs);
            Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_NUM);
            Cy_SysLib_Delay(delayMs);
        }

        /* Enter deep sleep mode */
        Cy_SysPm_CpuEnterDeepSleep();

    }
}

/* [] END OF FILE */
