/*
 * I2C.h
 *
 * AXI4-Lite custom I2C Master HAL.
 */

#ifndef SRC_HAL_I2C_I2C_H_
#define SRC_HAL_I2C_I2C_H_

#include <stdint.h>
#include "xparameters.h"

typedef struct {
    volatile uint32_t CONTROL;
    volatile uint32_t STATUS;
    volatile uint32_t TXDATA;
    volatile uint32_t RXDATA;
    volatile uint32_t CLK_DIV;
    volatile uint32_t SLAVE_ADDR;
    volatile uint32_t IRQ_ENABLE;
    volatile uint32_t IRQ_STATUS;
} I2C_TypeDef_t;

#if defined(XPAR_I2C_MASTER_0_S00_AXI_BASEADDR)
#define I2C_BASEADDR XPAR_I2C_MASTER_0_S00_AXI_BASEADDR
#elif defined(XPAR_I2C_LCD_0_S00_AXI_BASEADDR)
#define I2C_BASEADDR XPAR_I2C_LCD_0_S00_AXI_BASEADDR
#elif defined(XPAR_I2C_MASTER_0_BASEADDR)
#define I2C_BASEADDR XPAR_I2C_MASTER_0_BASEADDR
#else
#error "I2C base address define was not found. Regenerate xparameters.h after adding i2c_master_0."
#endif

#define I2C0 ((I2C_TypeDef_t *)I2C_BASEADDR)

#define I2C_CONTROL_START              0x00000001u
#define I2C_CONTROL_STOP               0x00000002u
#define I2C_CONTROL_RW                 0x00000004u
#define I2C_CONTROL_IRQ_ENABLE         0x00000008u
#define I2C_CONTROL_ACK_IRQ_ENABLE     0x00000010u
#define I2C_CONTROL_SOFT_RESET         0x00000080u

#define I2C_STATUS_BUSY                0x00000001u
#define I2C_STATUS_DONE                0x00000002u
#define I2C_STATUS_ACK_ERROR           0x00000004u
#define I2C_STATUS_BUS_ERROR           0x00000008u
#define I2C_STATUS_IRQ_PENDING         0x00000010u

#define I2C_IRQ_DONE                   0x00000001u
#define I2C_IRQ_ACK_ERROR              0x00000002u
#define I2C_IRQ_BUS_ERROR              0x00000004u
#define I2C_IRQ_ALL                    0x00000007u

#define I2C_DEFAULT_SLAVE_ADDR         0x27u
#define I2C_ALT_SLAVE_ADDR             0x3Fu
#define I2C_DEFAULT_CLK_DIV            250u
#define I2C_DEFAULT_TIMEOUT_MS         20u

typedef enum {
    I2C_OK = 0,
    I2C_TIMEOUT = -1,
    I2C_ACK_ERROR = -2,
    I2C_BUS_ERROR = -3
} I2C_Status_t;

void I2C_Init(I2C_TypeDef_t *i2c);
void I2C_SetSlaveAddress(I2C_TypeDef_t *i2c, uint8_t addr);
void I2C_SetClockDivider(I2C_TypeDef_t *i2c, uint16_t clkDiv);
void I2C_EnableInterrupt(I2C_TypeDef_t *i2c, uint32_t mask);
void I2C_DisableInterrupt(I2C_TypeDef_t *i2c, uint32_t mask);
void I2C_ClearIrq(I2C_TypeDef_t *i2c, uint32_t mask);
void I2C_ClearFlags(void);
uint32_t I2C_GetStatus(I2C_TypeDef_t *i2c);
uint32_t I2C_GetIrqStatus(I2C_TypeDef_t *i2c);
uint8_t I2C_IsBusy(I2C_TypeDef_t *i2c);
I2C_Status_t I2C_WriteByte(I2C_TypeDef_t *i2c, uint8_t data, uint32_t timeoutMs);
I2C_Status_t I2C_WriteBuffer(I2C_TypeDef_t *i2c, const uint8_t *data, uint32_t len, uint32_t timeoutMs);
void I2C_IRQHandler(I2C_TypeDef_t *i2c);
uint8_t I2C_IsTransferDone(void);
uint8_t I2C_HasError(void);
uint8_t I2C_HasAckError(void);
uint8_t I2C_HasBusError(void);

#endif /* SRC_HAL_I2C_I2C_H_ */

