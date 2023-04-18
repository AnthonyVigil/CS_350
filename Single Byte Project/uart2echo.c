/*
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART2.h>

/* Driver configuration */
#include "ti_drivers_config.h"


/* UART Variables */
UART2_Handle uart;
UART2_Params uartParams;
size_t bytesRead;
size_t bytesWritten = 0;
uint32_t status = UART2_STATUS_SUCCESS;

/* Function prototypes */
void turnOnLED();
void turnOffLED();

/*
* ======== mainThread ========
*/
void *mainThread(void *arg0)
{
    char input;
    const char echoPrompt[] = "Echoing characters:\r\n";

    /* Call driver init functions */
    GPIO_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Create a UART where the default read and write mode is BLOCKING */
    UART2_Params_init(&uartParams);
    uartParams.writeMode = UART2_Mode_BLOCKING;
    uartParams.readMode = UART2_Mode_BLOCKING;
    uartParams.baudRate = 115200;

    uart = UART2_open(CONFIG_UART2_0, &uartParams);

    if (uart == NULL) {
        /* UART2_open() failed */
        while (1);
    }

    /* Turn on user LED to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    UART2_write(uart, echoPrompt, sizeof(echoPrompt), &bytesWritten);

    enum LED_States {Start, NorF, F2, LED_on, LED_off} LED_State;

    /* State Machine Loop */
    while (1) {
        UART2_read(uart, &input, 1, &bytesRead);    // Read Input

        // State Transitions
        switch (LED_State) {

            // Start Case
            case Start:

                // If input is o, transition to NorF
                if (input == 'O'){
                    LED_State = NorF;
                }
                break;

            // NorF Case, searching for user input of 'n' or 'f'
            case NorF:

                // N found, transition to LED_on
                if (input =='N'){
                    LED_State = LED_on;
                }

                // F Found, transition to F2
                else if (input == 'F'){
                    LED_State = F2;
                }
                break;

            // F2 Case
            case F2:

                // If user input is f, update LED_State to off
                if (input == 'F'){
                    LED_State = LED_off;
                }
                break;

            // Default state transition to start
            default:
                LED_State = Start;
                break;
        }

       // State Action
       switch(LED_State){

            // LED On
            case LED_on:
                GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

                // Transition to Start
                LED_State = Start;
                break;

            // LED Off
            case LED_off:
                GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);

                // Transition to Start
                LED_State = Start;
                break;
        }

        // Write Input with Each Loop
        UART2_write(uart, &input, 1, &bytesRead);
    }
}
