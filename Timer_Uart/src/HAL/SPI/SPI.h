/*
 * SPI.h
 *
 * AXI4-Lite custom SPI Master HAL.
 */

#ifndef SRC_HAL_SPI_SPI_H_
#define SRC_HAL_SPI_SPI_H_

#include <stdint.h>
#include "xparameters.h"

typedef struct {
    volatile uint32_t STATUS;
    volatile uint32_t TDR;
    volatile uint32_t RDR;
    volatile uint32_t CR;
} SPI_TypeDef_t;

#if defined(XPAR_SPI_0_S00_AXI_BASEADDR)
#define SPI_BASEADDR XPAR_SPI_0_S00_AXI_BASEADDR
#elif defined(XPAR_SPI_0_BASEADDR)
#define SPI_BASEADDR XPAR_SPI_0_BASEADDR
#else
#define SPI_BASEADDR 0x44A70000u
#endif

#define SPI0 ((SPI_TypeDef_t *)SPI_BASEADDR)

#define SPI_STATUS_READY              0x00000001u
#define SPI_STATUS_DONE               0x00000002u
#define SPI_STATUS_WRITE_ERROR        0x00000004u

#define SPI_CR_RX_IE                  0x00000001u
#define SPI_CR_CLEAR_WRITE_ERROR      0x00000002u
#define SPI_CR_SS_HOLD                0x00000004u
#define SPI_CR_MASK                   (SPI_CR_RX_IE | SPI_CR_SS_HOLD)

#define SPI_DEFAULT_TIMEOUT_MS        20u

typedef enum {
    SPI_OK = 0,
    SPI_TIMEOUT = -1,
    SPI_BUSY = -2,
    SPI_WRITE_ERROR = -3
} SPI_Status_t;

void SPI_Init(SPI_TypeDef_t *spi);
void SPI_EnableInterrupt(SPI_TypeDef_t *spi);
void SPI_DisableInterrupt(SPI_TypeDef_t *spi);
void SPI_ClearWriteError(SPI_TypeDef_t *spi);
void SPI_SetSlaveSelect(SPI_TypeDef_t *spi, uint8_t active);
uint32_t SPI_GetStatus(SPI_TypeDef_t *spi);
uint8_t SPI_IsReady(SPI_TypeDef_t *spi);
SPI_Status_t SPI_TransferByte(SPI_TypeDef_t *spi, uint8_t txData, uint8_t *rxData, uint32_t timeoutMs);
SPI_Status_t SPI_WriteByte(SPI_TypeDef_t *spi, uint8_t data, uint32_t timeoutMs);
SPI_Status_t SPI_ReadByte(SPI_TypeDef_t *spi, uint8_t *data, uint32_t timeoutMs);

#endif /* SRC_HAL_SPI_SPI_H_ */

