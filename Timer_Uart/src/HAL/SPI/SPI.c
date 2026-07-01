/*
 * SPI.c
 *
 * AXI4-Lite custom SPI Master HAL.
 */

#include "SPI.h"
#include "../../common/delay/delay.h"

static uint32_t spiControlShadow;

static uint8_t SPI_IsTimedOut(uint32_t startTime, uint32_t timeoutMs)
{
    return (millis() - startTime) >= timeoutMs;
}

static void SPI_WriteControl(SPI_TypeDef_t *spi)
{
    spi->CR = spiControlShadow & SPI_CR_MASK;
}

void SPI_Init(SPI_TypeDef_t *spi)
{
    spiControlShadow = 0u;
    spi->CR = SPI_CR_CLEAR_WRITE_ERROR;
    spi->CR = 0u;
    if (spi->STATUS & SPI_STATUS_DONE) {
        (void)spi->RDR;
    }
}

void SPI_EnableInterrupt(SPI_TypeDef_t *spi)
{
    spiControlShadow |= SPI_CR_RX_IE;
    SPI_WriteControl(spi);
}

void SPI_DisableInterrupt(SPI_TypeDef_t *spi)
{
    spiControlShadow &= ~SPI_CR_RX_IE;
    SPI_WriteControl(spi);
}

void SPI_ClearWriteError(SPI_TypeDef_t *spi)
{
    spi->CR = (spiControlShadow & SPI_CR_MASK) | SPI_CR_CLEAR_WRITE_ERROR;
    SPI_WriteControl(spi);
}

void SPI_SetSlaveSelect(SPI_TypeDef_t *spi, uint8_t active)
{
    if (active) {
        spiControlShadow |= SPI_CR_SS_HOLD;
    }
    else {
        spiControlShadow &= ~SPI_CR_SS_HOLD;
    }
    SPI_WriteControl(spi);
}

uint32_t SPI_GetStatus(SPI_TypeDef_t *spi)
{
    return spi->STATUS;
}

uint8_t SPI_IsReady(SPI_TypeDef_t *spi)
{
    return (spi->STATUS & SPI_STATUS_READY) ? 1u : 0u;
}

SPI_Status_t SPI_TransferByte(SPI_TypeDef_t *spi, uint8_t txData, uint8_t *rxData, uint32_t timeoutMs)
{
    uint32_t startTime;
    uint32_t status;

    if (timeoutMs == 0u) {
        timeoutMs = SPI_DEFAULT_TIMEOUT_MS;
    }

    if (spi->STATUS & SPI_STATUS_WRITE_ERROR) {
        SPI_ClearWriteError(spi);
    }
    if (spi->STATUS & SPI_STATUS_DONE) {
        (void)spi->RDR;
    }

    startTime = millis();
    while (!SPI_IsReady(spi)) {
        if (SPI_IsTimedOut(startTime, timeoutMs)) {
            return SPI_BUSY;
        }
    }

    spi->TDR = txData;

    startTime = millis();
    do {
        status = spi->STATUS;
        if (status & SPI_STATUS_WRITE_ERROR) {
            SPI_ClearWriteError(spi);
            return SPI_WRITE_ERROR;
        }
        if (SPI_IsTimedOut(startTime, timeoutMs)) {
            return SPI_TIMEOUT;
        }
    } while ((status & SPI_STATUS_DONE) == 0u);

    if (rxData != 0) {
        *rxData = (uint8_t)(spi->RDR & 0xFFu);
    }
    else {
        (void)spi->RDR;
    }

    return SPI_OK;
}

SPI_Status_t SPI_WriteByte(SPI_TypeDef_t *spi, uint8_t data, uint32_t timeoutMs)
{
    return SPI_TransferByte(spi, data, 0, timeoutMs);
}

SPI_Status_t SPI_ReadByte(SPI_TypeDef_t *spi, uint8_t *data, uint32_t timeoutMs)
{
    return SPI_TransferByte(spi, 0x00u, data, timeoutMs);
}
