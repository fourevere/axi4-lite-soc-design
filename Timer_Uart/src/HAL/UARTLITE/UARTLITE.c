#include "UARTLITE.h"
#include "xuartlite_l.h"

uint8_t UARTLITE_RxAvailable(void)
{
    return (XUartLite_IsReceiveEmpty(UARTLITE_BASEADDR) == 0) ? 1u : 0u;
}

uint8_t UARTLITE_Receive(void)
{
    return XUartLite_RecvByte(UARTLITE_BASEADDR);
}

void UARTLITE_Transmit(uint8_t data)
{
    while (XUartLite_IsTransmitFull(UARTLITE_BASEADDR) != 0) {
    }
    XUartLite_SendByte(UARTLITE_BASEADDR, data);
}

void UARTLITE_TransmitString(const char *str)
{
    while (str != 0 && *str != '\0') {
        UARTLITE_Transmit((uint8_t)(*str));
        str++;
    }
}
