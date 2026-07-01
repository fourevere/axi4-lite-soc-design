#include "TimeSetting.h"
#include "../Clock/Clock.h"
#include "../../driver/LCD1602_I2C/LCD1602_I2C.h"
#include "xil_printf.h"

#define TIME_SETTING_BUFFER_SIZE 16u

static uint8_t timeSettingEnabled;
static char timeSettingBuffer[TIME_SETTING_BUFFER_SIZE + 1u];
static uint8_t timeSettingIndex;

static void TimeSetting_FillLine(char *line)
{
    uint8_t index;

    for (index = 0u; index < 16u; index++) {
        line[index] = ' ';
    }
    line[16] = '\0';
}

static void TimeSetting_CopyText(char *line, uint8_t pos, const char *text)
{
    uint8_t index = 0u;

    while (text != 0 && text[index] != '\0' && pos < 16u) {
        line[pos] = text[index];
        pos++;
        index++;
    }
}

static void TimeSetting_PrintLine(uint8_t row, const char *text)
{
    char line[17];

    TimeSetting_FillLine(line);
    TimeSetting_CopyText(line, 0u, text);
    (void)lcd1602_i2c_set_cursor(row, 0u);
    (void)lcd1602_i2c_puts(line);
}

static uint8_t TimeSetting_IsDigit(char data)
{
    return (data >= '0' && data <= '9') ? 1u : 0u;
}

static uint8_t TimeSetting_ParseTime(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
    uint8_t parsedHour;
    uint8_t parsedMin;
    uint8_t parsedSec;

    if (timeSettingIndex != 10u) return 0u;
    if (timeSettingBuffer[0] != 'T' && timeSettingBuffer[0] != 't') return 0u;
    if (timeSettingBuffer[1] != '=') return 0u;
    if (timeSettingBuffer[4] != ':' || timeSettingBuffer[7] != ':') return 0u;
    if (TimeSetting_IsDigit(timeSettingBuffer[2]) == 0u) return 0u;
    if (TimeSetting_IsDigit(timeSettingBuffer[3]) == 0u) return 0u;
    if (TimeSetting_IsDigit(timeSettingBuffer[5]) == 0u) return 0u;
    if (TimeSetting_IsDigit(timeSettingBuffer[6]) == 0u) return 0u;
    if (TimeSetting_IsDigit(timeSettingBuffer[8]) == 0u) return 0u;
    if (TimeSetting_IsDigit(timeSettingBuffer[9]) == 0u) return 0u;

    parsedHour = (uint8_t)((timeSettingBuffer[2] - '0') * 10 + (timeSettingBuffer[3] - '0'));
    parsedMin = (uint8_t)((timeSettingBuffer[5] - '0') * 10 + (timeSettingBuffer[6] - '0'));
    parsedSec = (uint8_t)((timeSettingBuffer[8] - '0') * 10 + (timeSettingBuffer[9] - '0'));

    if (parsedHour > 23u || parsedMin > 59u || parsedSec > 59u) {
        return 0u;
    }

    *hour = parsedHour;
    *min = parsedMin;
    *sec = parsedSec;
    return 1u;
}

static void TimeSetting_ResetBuffer(void)
{
    uint8_t index;

    for (index = 0u; index <= TIME_SETTING_BUFFER_SIZE; index++) {
        timeSettingBuffer[index] = '\0';
    }
    timeSettingIndex = 0u;
}

static void TimeSetting_ShowInput(void)
{
    if (timeSettingIndex == 0u) {
        TimeSetting_PrintLine(1u, "T=12:34:56");
    }
    else {
        TimeSetting_PrintLine(1u, timeSettingBuffer);
    }
}

static void TimeSetting_Commit(void)
{
    uint8_t hour;
    uint8_t min;
    uint8_t sec;

    if (TimeSetting_ParseTime(&hour, &min, &sec) != 0u) {
        Clock_SetTime(hour, min, sec);
        TimeSetting_PrintLine(0u, "Time Updated");
        TimeSetting_PrintLine(1u, timeSettingBuffer);
        xil_printf("TIME SET %s\r\n", timeSettingBuffer);
    }
    else {
        TimeSetting_PrintLine(0u, "Invalid Format");
        TimeSetting_PrintLine(1u, "T=12:34:56");
        xil_printf("TIME SET ERROR: use T=HH:MM:SS\r\n");
    }

    TimeSetting_ResetBuffer();
}

void TimeSetting_Init(void)
{
    timeSettingEnabled = 0u;
    TimeSetting_ResetBuffer();
}

void TimeSetting_Enable(void)
{
    timeSettingEnabled = 1u;
    TimeSetting_ResetBuffer();
    (void)lcd1602_i2c_clear();
    TimeSetting_PrintLine(0u, "Set Time");
    TimeSetting_PrintLine(1u, "T=12:34:56");
    xil_printf("MODE Time Setting\r\n");
    xil_printf("Type T=HH:MM:SS and press Enter.\r\n");
}

void TimeSetting_Disable(void)
{
    timeSettingEnabled = 0u;
}

void TimeSetting_OnChar(uint8_t data)
{
    if (timeSettingEnabled == 0u) {
        return;
    }

    if (data == '\r' || data == '\n') {
        xil_printf("\r\n");
        TimeSetting_Commit();
        return;
    }

    if (data == 0x08u || data == 0x7Fu) {
        if (timeSettingIndex != 0u) {
            timeSettingIndex--;
            timeSettingBuffer[timeSettingIndex] = '\0';
            xil_printf("\b \b");
            TimeSetting_ShowInput();
        }
        return;
    }

    if (data < 0x20u || data > 0x7Eu) {
        return;
    }

    if (timeSettingIndex >= TIME_SETTING_BUFFER_SIZE) {
        return;
    }

    timeSettingBuffer[timeSettingIndex] = (char)data;
    timeSettingIndex++;
    timeSettingBuffer[timeSettingIndex] = '\0';
    xil_printf("%c", data);
    TimeSetting_ShowInput();
}
