/*
 * LCD1602_I2C.c
 *
 * LCD1602 driver through a PCF8574 I2C backpack.
 */

#include "LCD1602_I2C.h"
#include "../../common/delay/delay.h"

static uint8_t lcdBacklight = LCD_PCF8574_BL;

static uint8_t lcd1602_i2c_pack_nibble(uint8_t nibble, uint8_t control)
{
    return ((nibble & 0x0Fu) << 4) | control | lcdBacklight;
}

static I2C_Status_t lcd1602_i2c_write_pcf8574(uint8_t data)
{
    return I2C_WriteByte(I2C0, data, I2C_DEFAULT_TIMEOUT_MS);
}

static I2C_Status_t lcd1602_i2c_write4(uint8_t nibble, uint8_t control)
{
    I2C_Status_t status;
    uint8_t value = lcd1602_i2c_pack_nibble(nibble, control);

    status = lcd1602_i2c_write_pcf8574(value | LCD_PCF8574_EN);
    if (status != I2C_OK) {
        return status;
    }
    delay_us(1);

    status = lcd1602_i2c_write_pcf8574(value & (uint8_t)~LCD_PCF8574_EN);
    if (status != I2C_OK) {
        return status;
    }
    delay_us(50);

    return I2C_OK;
}

static I2C_Status_t lcd1602_i2c_write_byte(uint8_t value, uint8_t control)
{
    I2C_Status_t status;

    status = lcd1602_i2c_write4(value >> 4, control);
    if (status != I2C_OK) {
        return status;
    }
    return lcd1602_i2c_write4(value & 0x0Fu, control);
}

I2C_Status_t lcd1602_i2c_init(void)
{
    I2C_Status_t status;

    delay_ms(50);

    status = lcd1602_i2c_write4(0x03u, 0u);
    if (status != I2C_OK) return status;
    delay_ms(5);

    status = lcd1602_i2c_write4(0x03u, 0u);
    if (status != I2C_OK) return status;
    delay_us(150);

    status = lcd1602_i2c_write4(0x03u, 0u);
    if (status != I2C_OK) return status;

    status = lcd1602_i2c_write4(0x02u, 0u);
    if (status != I2C_OK) return status;

    status = lcd1602_i2c_write_command(0x28u);
    if (status != I2C_OK) return status;
    status = lcd1602_i2c_write_command(0x0Cu);
    if (status != I2C_OK) return status;
    status = lcd1602_i2c_write_command(0x06u);
    if (status != I2C_OK) return status;
    return lcd1602_i2c_clear();
}

I2C_Status_t lcd1602_i2c_clear(void)
{
    I2C_Status_t status = lcd1602_i2c_write_command(0x01u);
    delay_ms(2);
    return status;
}

I2C_Status_t lcd1602_i2c_set_cursor(uint8_t row, uint8_t col)
{
    static const uint8_t rowOffset[2] = {0x00u, 0x40u};

    if (row > 1u) {
        row = 1u;
    }
    if (col > 15u) {
        col = 15u;
    }

    return lcd1602_i2c_write_command(0x80u | (rowOffset[row] + col));
}

I2C_Status_t lcd1602_i2c_write_command(uint8_t cmd)
{
    return lcd1602_i2c_write_byte(cmd, 0u);
}

I2C_Status_t lcd1602_i2c_write_data(uint8_t data)
{
    return lcd1602_i2c_write_byte(data, LCD_PCF8574_RS);
}

I2C_Status_t lcd1602_i2c_putc(char ch)
{
    return lcd1602_i2c_write_data((uint8_t)ch);
}

I2C_Status_t lcd1602_i2c_puts(const char *str)
{
    I2C_Status_t status;

    while (*str) {
        status = lcd1602_i2c_putc(*str);
        if (status != I2C_OK) {
            return status;
        }
        str++;
    }
    return I2C_OK;
}

void lcd1602_i2c_backlight_on(void)
{
    lcdBacklight = LCD_PCF8574_BL;
}

void lcd1602_i2c_backlight_off(void)
{
    lcdBacklight = 0u;
}

I2C_Status_t lcd1602_i2c_print_time(const stopWatch_t *timeData)
{
    char timeText[11];
    I2C_Status_t status;

    StopWatch_FormatTimeTenth(timeData, timeText);

    status = lcd1602_i2c_clear();
    if (status != I2C_OK) return status;
    status = lcd1602_i2c_set_cursor(0u, 0u);
    if (status != I2C_OK) return status;
    status = lcd1602_i2c_puts("Current Time");
    if (status != I2C_OK) return status;
    status = lcd1602_i2c_set_cursor(1u, 0u);
    if (status != I2C_OK) return status;
    return lcd1602_i2c_puts(timeText);
}
