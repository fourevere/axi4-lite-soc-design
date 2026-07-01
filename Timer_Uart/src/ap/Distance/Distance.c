#include "Distance.h"
#include "../../common/delay/delay.h"
#include "../../driver/Button/Button.h"
#include "../../driver/LCD1602_I2C/LCD1602_I2C.h"
#include "../../driver/SR04/SR04.h"
#include "xil_printf.h"

static uint8_t distanceEnabled;

static void Distance_FillLine(char *line)
{
    uint8_t index;

    for (index = 0u; index < 16u; index++) {
        line[index] = ' ';
    }
    line[16] = '\0';
}

static void Distance_CopyText(char *line, uint8_t pos, const char *text)
{
    uint8_t index = 0u;

    while (text != 0 && text[index] != '\0' && pos < 16u) {
        line[pos] = text[index];
        pos++;
        index++;
    }
}

static void Distance_PrintLine(uint8_t row, const char *text)
{
    char line[17];

    Distance_FillLine(line);
    Distance_CopyText(line, 0u, text);
    (void)lcd1602_i2c_set_cursor(row, 0u);
    (void)lcd1602_i2c_puts(line);
}

static uint8_t Distance_WriteNumber(char *line, uint8_t pos, uint32_t value)
{
    char digits[10];
    uint8_t count = 0u;

    if (value == 0u) {
        line[pos] = '0';
        return (uint8_t)(pos + 1u);
    }

    while (value != 0u && count < 10u) {
        digits[count] = (char)('0' + (value % 10u));
        value /= 10u;
        count++;
    }

    while (count != 0u && pos < 16u) {
        count--;
        line[pos] = digits[count];
        pos++;
    }

    return pos;
}

static void Distance_PrintDistance(uint32_t distanceMm)
{
    char line[17];
    uint8_t pos;

    if (distanceMm == 0u) {
        Distance_FillLine(line);
        Distance_CopyText(line, 0u, "No Echo E");
        line[9] = (SR04_ReadEchoPin() != 0u) ? '1' : '0';
        (void)lcd1602_i2c_set_cursor(1u, 0u);
        (void)lcd1602_i2c_puts(line);
        return;
    }

    Distance_FillLine(line);
    Distance_CopyText(line, 0u, "Dist ");
    pos = Distance_WriteNumber(line, 5u, distanceMm);
    Distance_CopyText(line, (uint8_t)(pos + 1u), "mm");
    (void)lcd1602_i2c_set_cursor(1u, 0u);
    (void)lcd1602_i2c_puts(line);
}

void Distance_Init(void)
{
    distanceEnabled = 0u;
    SR04_Init();
}

void Distance_Enable(void)
{
    distanceEnabled = 1u;
    (void)lcd1602_i2c_clear();
    Distance_PrintLine(0u, "SR04 Distance");
    Distance_PrintLine(1u, "Press BTNU");
    xil_printf("MODE SR04 Distance\r\n");
    xil_printf("Press BTNU to measure SR04 distance.\r\n");
}

void Distance_Disable(void)
{
    distanceEnabled = 0u;
}

void Distance_Excute(void)
{
    uint32_t distanceMm;

    if (distanceEnabled == 0u) {
        return;
    }

    if (Button_GetState(&hbtnRunStop) != ACT_PUSHED) {
        return;
    }

    xil_printf("Measure\r\n");
    distanceMm = SR04_ReadDistanceMmFiltered();
    Distance_PrintDistance(distanceMm);

    if (distanceMm == 0u) {
        xil_printf("SR04 Timeout CR=0x%x IDR=0x%x ODR=0x%x ECHO=%d\r\n",
                   GPIO_GetCR(SR04_GPIO),
                   GPIO_ReadPort(SR04_GPIO),
                   GPIO_GetODR(SR04_GPIO),
                   SR04_ReadEchoPin());
    }
    else {
        xil_printf("Distance = %lu.%lu cm\r\n",
                   distanceMm / 10u,
                   distanceMm % 10u);
    }
}
