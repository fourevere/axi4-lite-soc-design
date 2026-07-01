/*
 * LCD1602_I2C.h
 *
 * LCD1602 driver through a PCF8574 I2C backpack.
 */

#ifndef SRC_DRIVER_LCD1602_I2C_LCD1602_I2C_H_
#define SRC_DRIVER_LCD1602_I2C_LCD1602_I2C_H_

#include <stdint.h>
#include "../../HAL/I2C/I2C.h"
#include "../../ap/StopWatch.h"

#define LCD_PCF8574_RS      0x01u
#define LCD_PCF8574_RW      0x02u
#define LCD_PCF8574_EN      0x04u
#define LCD_PCF8574_BL      0x08u
#define LCD_PCF8574_D4      0x10u
#define LCD_PCF8574_D5      0x20u
#define LCD_PCF8574_D6      0x40u
#define LCD_PCF8574_D7      0x80u

#define LCD1602_I2C_ADDR_DEFAULT I2C_DEFAULT_SLAVE_ADDR
#define LCD1602_I2C_ADDR_ALT     I2C_ALT_SLAVE_ADDR

I2C_Status_t lcd1602_i2c_init(void);
I2C_Status_t lcd1602_i2c_clear(void);
I2C_Status_t lcd1602_i2c_set_cursor(uint8_t row, uint8_t col);
I2C_Status_t lcd1602_i2c_write_command(uint8_t cmd);
I2C_Status_t lcd1602_i2c_write_data(uint8_t data);
I2C_Status_t lcd1602_i2c_putc(char ch);
I2C_Status_t lcd1602_i2c_puts(const char *str);
void lcd1602_i2c_backlight_on(void);
void lcd1602_i2c_backlight_off(void);
I2C_Status_t lcd1602_i2c_print_time(const stopWatch_t *timeData);

#endif /* SRC_DRIVER_LCD1602_I2C_LCD1602_I2C_H_ */
