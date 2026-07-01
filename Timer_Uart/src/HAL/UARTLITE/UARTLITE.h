#ifndef SRC_HAL_UARTLITE_UARTLITE_H_
#define SRC_HAL_UARTLITE_UARTLITE_H_

#include <stdint.h>
#include "xparameters.h"

#define UARTLITE_BASEADDR XPAR_AXI_UARTLITE_0_BASEADDR

uint8_t UARTLITE_RxAvailable(void);
uint8_t UARTLITE_Receive(void);
void UARTLITE_Transmit(uint8_t data);
void UARTLITE_TransmitString(const char *str);

#endif /* SRC_HAL_UARTLITE_UARTLITE_H_ */
