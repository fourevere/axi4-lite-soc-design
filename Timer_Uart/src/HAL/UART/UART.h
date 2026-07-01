/*
 * UART.h
 *
 *  Created on: 2026. 6. 26.
 *      Author: kccistc
 */

#ifndef SRC_HAL_UART_UART_H_
#define SRC_HAL_UART_UART_H_

#include <stdint.h>
#include "xparameters.h"

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t TDR;
    volatile uint32_t RDR;
    volatile uint32_t CR;
}UART_TypeDef_t;

#define UART_BASEADDR  XPAR_UART_0_S00_AXI_BASEADDR
#define UART0          (UART_TypeDef_t *)UART_BASEADDR

void UART_StartInterrupt(UART_TypeDef_t *uart);
void UART_StopInterrupt(UART_TypeDef_t *uart);
void UART_Transmit(UART_TypeDef_t *uart, uint8_t data);
void UART_TransmitString(UART_TypeDef_t *uart, const char *str);
uint8_t UART_Receive(UART_TypeDef_t *uart);
uint8_t UART_RxAvalable(UART_TypeDef_t *uart);

#endif /* SRC_HAL_UART_UART_H_ */

