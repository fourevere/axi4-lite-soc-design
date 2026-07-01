/*
 * UART.c
 *
 *  Created on: 2026. 6. 26.
 *      Author: kccistc
 */
#include "UART.h"

void UART_StartInterrupt(UART_TypeDef_t *uart)
{
    uart->CR |= 1<<0;
}

void UART_StopInterrupt(UART_TypeDef_t *uart)
{
    uart->CR &= ~(1<<0);
}

void UART_Transmit(UART_TypeDef_t *uart, uint8_t data)
{
    while ((uart->SR & (1u << 0)) == 0u) {
    }
    uart->TDR = (uint32_t)data;
}

void UART_TransmitString(UART_TypeDef_t *uart, const char *str)
{
    while (*str) {
        UART_Transmit(uart, (uint8_t)(*str));
        str++;
    }
}

uint8_t UART_Receive(UART_TypeDef_t *uart)
{
    while ((uart->SR & (1u << 1)) == 0u) {
    }
    return (uint8_t)(uart->RDR);
}

uint8_t UART_RxAvalable(UART_TypeDef_t *uart)
{
    return uart->SR & (1<<1);
}

