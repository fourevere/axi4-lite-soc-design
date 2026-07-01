/*
 * I2C.c
 *
 * AXI4-Lite custom I2C Master HAL.
 */

#include "I2C.h"
#include "../../common/delay/delay.h"

static volatile uint8_t i2cTransferDone;
static volatile uint8_t i2cAckError;
static volatile uint8_t i2cBusError;

void I2C_Init(I2C_TypeDef_t *i2c)
{
    I2C_ClearFlags();
    i2c->CONTROL = I2C_CONTROL_SOFT_RESET;
    i2c->CLK_DIV = I2C_DEFAULT_CLK_DIV;
    i2c->SLAVE_ADDR = I2C_DEFAULT_SLAVE_ADDR;
    i2c->IRQ_STATUS = I2C_IRQ_ALL;
    i2c->IRQ_ENABLE = I2C_IRQ_DONE | I2C_IRQ_ACK_ERROR | I2C_IRQ_BUS_ERROR;
}

void I2C_SetSlaveAddress(I2C_TypeDef_t *i2c, uint8_t addr)
{
    i2c->SLAVE_ADDR = addr & 0x7Fu;
}

void I2C_SetClockDivider(I2C_TypeDef_t *i2c, uint16_t clkDiv)
{
    i2c->CLK_DIV = clkDiv;
}

void I2C_EnableInterrupt(I2C_TypeDef_t *i2c, uint32_t mask)
{
    i2c->IRQ_ENABLE |= (mask & I2C_IRQ_ALL);
}

void I2C_DisableInterrupt(I2C_TypeDef_t *i2c, uint32_t mask)
{
    i2c->IRQ_ENABLE &= ~(mask & I2C_IRQ_ALL);
}

void I2C_ClearIrq(I2C_TypeDef_t *i2c, uint32_t mask)
{
    i2c->IRQ_STATUS = (mask & I2C_IRQ_ALL);
}

void I2C_ClearFlags(void)
{
    i2cTransferDone = 0;
    i2cAckError = 0;
    i2cBusError = 0;
}

uint32_t I2C_GetStatus(I2C_TypeDef_t *i2c)
{
    return i2c->STATUS;
}

uint32_t I2C_GetIrqStatus(I2C_TypeDef_t *i2c)
{
    return i2c->IRQ_STATUS;
}

uint8_t I2C_IsBusy(I2C_TypeDef_t *i2c)
{
    return (i2c->STATUS & I2C_STATUS_BUSY) ? 1u : 0u;
}

static uint8_t I2C_IsTimedOut(uint32_t startTime, uint32_t timeoutMs)
{
    return (millis() - startTime) >= timeoutMs;
}

I2C_Status_t I2C_WriteByte(I2C_TypeDef_t *i2c, uint8_t data, uint32_t timeoutMs)
{
    uint32_t startTime;

    if (timeoutMs == 0u) {
        timeoutMs = I2C_DEFAULT_TIMEOUT_MS;
    }

    startTime = millis();
    while (I2C_IsBusy(i2c)) {
        if (I2C_IsTimedOut(startTime, timeoutMs)) {
            return I2C_TIMEOUT;
        }
    }

    I2C_ClearFlags();
    I2C_ClearIrq(i2c, I2C_IRQ_ALL);
    i2c->TXDATA = data;
    i2c->CONTROL = I2C_CONTROL_START |
                   I2C_CONTROL_STOP |
                   I2C_CONTROL_IRQ_ENABLE |
                   I2C_CONTROL_ACK_IRQ_ENABLE;

    startTime = millis();
    while (!I2C_IsTransferDone() && !I2C_HasError()) {
        if (I2C_IsTimedOut(startTime, timeoutMs)) {
            return I2C_TIMEOUT;
        }
    }

    if (I2C_HasAckError()) {
        return I2C_ACK_ERROR;
    }
    if (I2C_HasBusError()) {
        return I2C_BUS_ERROR;
    }
    return I2C_OK;
}

I2C_Status_t I2C_WriteBuffer(I2C_TypeDef_t *i2c, const uint8_t *data, uint32_t len, uint32_t timeoutMs)
{
    uint32_t index;
    I2C_Status_t status;

    for (index = 0; index < len; index++) {
        status = I2C_WriteByte(i2c, data[index], timeoutMs);
        if (status != I2C_OK) {
            return status;
        }
    }
    return I2C_OK;
}

void I2C_IRQHandler(I2C_TypeDef_t *i2c)
{
    uint32_t irqStatus = I2C_GetIrqStatus(i2c);
    uint32_t status = I2C_GetStatus(i2c);

    if (irqStatus & I2C_IRQ_DONE) {
        i2cTransferDone = 1;
    }
    if ((irqStatus & I2C_IRQ_ACK_ERROR) || (status & I2C_STATUS_ACK_ERROR)) {
        i2cAckError = 1;
    }
    if ((irqStatus & I2C_IRQ_BUS_ERROR) || (status & I2C_STATUS_BUS_ERROR)) {
        i2cBusError = 1;
    }

    I2C_ClearIrq(i2c, irqStatus);
}

uint8_t I2C_IsTransferDone(void)
{
    return i2cTransferDone;
}

uint8_t I2C_HasError(void)
{
    return i2cAckError || i2cBusError;
}

uint8_t I2C_HasAckError(void)
{
    return i2cAckError;
}

uint8_t I2C_HasBusError(void)
{
    return i2cBusError;
}
