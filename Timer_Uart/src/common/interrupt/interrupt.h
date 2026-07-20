/*
 * interrupt.h
 *
 *  Created on: 2026. 6. 26.
 *      Author: kccistc
 */

#ifndef SRC_COMMON_INTERRUPT_INTERRUPT_H_
#define SRC_COMMON_INTERRUPT_INTERRUPT_H_

#include "xparameters.h"
#include "xintc.h"
#include "xil_exception.h"

#define INTC_DEV_ID     XPAR_INTC_0_DEVICE_ID

#define TMR_VEC_ID      XPAR_INTC_0_TIMER_0_VEC_ID
#define UART_VEC_ID     XPAR_INTC_0_UART_0_VEC_ID

#if defined(XPAR_INTC_0_I2C_MASTER_0_VEC_ID)
#define I2C_VEC_ID      XPAR_INTC_0_I2C_MASTER_0_VEC_ID
#elif defined(XPAR_INTC_0_I2C_LCD_0_VEC_ID)
#define I2C_VEC_ID      XPAR_INTC_0_I2C_LCD_0_VEC_ID
#elif defined(XPAR_MICROBLAZE_0_AXI_INTC_I2C_MASTER_0_INTR_INTR)
#define I2C_VEC_ID      XPAR_MICROBLAZE_0_AXI_INTC_I2C_MASTER_0_INTR_INTR
#elif defined(XPAR_MICROBLAZE_0_AXI_INTC_I2C_MASTER_0_INTR)
#define I2C_VEC_ID      XPAR_MICROBLAZE_0_AXI_INTC_I2C_MASTER_0_INTR
#else
#error "I2C interrupt vector define was not found. Regenerate xparameters.h after connecting i2c_master_0/intr."
#endif

void TMR_ISR(void *CallbackRef);
void UART_ISR(void *CallbackRef);
void I2C_ISR(void *CallbackRef);
int SetupInterruptSystem();

#endif /* SRC_COMMON_INTERRUPT_INTERRUPT_H_ */
