#include "LcdTime.h"
#include "../Clock/Clock.h"
#include "../../common/delay/delay.h"
#include "../../driver/LCD1602_I2C/LCD1602_I2C.h"
#include "xil_printf.h"

#define LCD_TIME_UPDATE_PERIOD_MS 100u

static uint8_t lcdTimeEnabled;
static uint32_t lcdTimePrevUpdate;

static void LcdTime_CopyText(char *line, const char *text)
{
    uint8_t index;

    for (index = 0u; index < 16u; index++) {
        line[index] = ' ';
    }
    line[16] = '\0';

    index = 0u;
    while (text != 0 && text[index] != '\0' && index < 16u) {
        line[index] = text[index];
        index++;
    }
}

static I2C_Status_t LcdTime_PrintHeader(void)
{
    char line[17];
    I2C_Status_t status;

    LcdTime_CopyText(line, "Current Time");

    status = lcd1602_i2c_clear();
    if (status != I2C_OK) return status;
    status = lcd1602_i2c_set_cursor(0u, 0u);
    if (status != I2C_OK) return status;
    return lcd1602_i2c_puts(line);
}

static I2C_Status_t LcdTime_PrintTimeLine(void)
{
    ClockTime_t timeData;
    char timeText[11];
    I2C_Status_t status;

    Clock_GetTime(&timeData);
    Clock_FormatTimeTenth(&timeData, timeText);

    status = lcd1602_i2c_set_cursor(1u, 0u);
    if (status != I2C_OK) return status;
    return lcd1602_i2c_puts(timeText);
}

void LcdTime_Init(void)
{
    I2C_Status_t status;

    lcdTimeEnabled = 0u;
    lcdTimePrevUpdate = 0u;

    status = lcd1602_i2c_init();
    if (status == I2C_OK) {
        xil_printf("LCD1602 I2C ready\r\n");
    }
    else {
        xil_printf("LCD1602 I2C init failed\r\n");
    }
}

void LcdTime_Enable(void)
{
    I2C_Status_t status;

    status = LcdTime_PrintHeader();
    if (status == I2C_OK) {
        lcdTimeEnabled = 1u;
        lcdTimePrevUpdate = millis();
        status = LcdTime_PrintTimeLine();
    }

    if (status == I2C_OK) {
        xil_printf("MODE Current Time\r\n");
    }
    else {
        xil_printf("LCD I2C status=%d\r\n", (int)status);
    }
}

void LcdTime_Disable(void)
{
    lcdTimeEnabled = 0u;
}

void LcdTime_Excute(void)
{
    uint32_t curTime;

    if (lcdTimeEnabled == 0u) {
        return;
    }

    curTime = millis();
    if ((curTime - lcdTimePrevUpdate) < LCD_TIME_UPDATE_PERIOD_MS) {
        return;
    }

    lcdTimePrevUpdate = curTime;
    LcdTime_PrintCurrentTime();
}

void LcdTime_PrintCurrentTime(void)
{
    I2C_Status_t status = LcdTime_PrintTimeLine();

    if (status != I2C_OK) {
        xil_printf("LCD I2C status=%d\r\n", (int)status);
    }
}
