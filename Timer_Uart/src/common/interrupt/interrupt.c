/*
 * interrupt.c
 *
 *  Created on: 2026. 6. 26.
 *      Author: kccistc
 */
#include "Interrupt.h"
#include "../delay/delay.h"
#include "../../driver/FND/FND.h"
#include "../../driver/LED/LED.h"
#include "../../HAL/UART/UART.h"
#include "../../HAL/I2C/I2C.h"

XIntc IntrController;
extern volatile uint8_t rx_data;

void TMR_ISR(void *CallbackRef)
{
    FND_Excute();
    incTick();
}

void UART_ISR(void *CallbackRef)
{
    if (UART_RxAvalable(UART0) != 0u) {
        rx_data = UART_Receive(UART0);
    }
}

void I2C_ISR(void *CallbackRef)
{
    I2C_IRQHandler(I2C0);
}

int SetupInterrupSystem()
{
    int status;

    status = XIntc_Initialize(&IntrController, INTC_DEV_ID);
    if(status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    status = XIntc_Connect(&IntrController, TMR_VEC_ID, (XInterruptHandler)TMR_ISR,(void *)0);
    if(status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    status = XIntc_Connect(&IntrController, UART_VEC_ID, (XInterruptHandler)UART_ISR,(void *)0);
    if(status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    status = XIntc_Connect(&IntrController, I2C_VEC_ID, (XInterruptHandler)I2C_ISR,(void *)0);
    if(status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    status = XIntc_Start(&IntrController, XIN_REAL_MODE);
    if(status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    XIntc_Enable(&IntrController, TMR_VEC_ID);
    XIntc_Enable(&IntrController, UART_VEC_ID);
    XIntc_Enable(&IntrController, I2C_VEC_ID);

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XIntc_InterruptHandler, &IntrController);
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

