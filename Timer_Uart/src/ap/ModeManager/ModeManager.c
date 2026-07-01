#include "ModeManager.h"
#include "../Attendance/Attendance.h"
#include "../Clock/Clock.h"
#include "../Distance/Distance.h"
#include "../LcdTime/LcdTime.h"
#include "../StopWatch.h"
#include "../TimeSetting/TimeSetting.h"
#include "../../common/delay/delay.h"
#include "../../driver/Button/Button.h"
#include "../../driver/LCD1602_I2C/LCD1602_I2C.h"
#include "../../driver/ModeSwitch/ModeSwitch.h"
#include "../../HAL/UART/UART.h"
#include "../../HAL/UARTLITE/UARTLITE.h"
#include "xil_printf.h"

#define STOPWATCH_LCD_UPDATE_PERIOD_MS 100u

static SystemMode_t currentMode;
static uint8_t modeInitialized;
static uint32_t stopwatchLcdPrevUpdate;

static void ModeManager_FillLine(char *line)
{
    uint8_t index;

    for (index = 0u; index < 16u; index++) {
        line[index] = ' ';
    }
    line[16] = '\0';
}

static void ModeManager_CopyText(char *line, uint8_t pos, const char *text)
{
    uint8_t index = 0u;

    while (text != 0 && text[index] != '\0' && pos < 16u) {
        line[pos] = text[index];
        pos++;
        index++;
    }
}

static void ModeManager_PrintLine(uint8_t row, const char *text)
{
    char line[17];

    ModeManager_FillLine(line);
    ModeManager_CopyText(line, 0u, text);
    (void)lcd1602_i2c_set_cursor(row, 0u);
    (void)lcd1602_i2c_puts(line);
}

static SystemMode_t ModeManager_ReadSwitchMode(void)
{
    uint8_t mode = ModeSwitch_Read();

    if (mode <= (uint8_t)MODE_TIME_SETTING) {
        return (SystemMode_t)mode;
    }
    return MODE_RESERVED;
}

static void ModeManager_DisableLcdOwners(void)
{
    LcdTime_Disable();
    Attendance_Disable();
    Distance_Disable();
    TimeSetting_Disable();
}

static void ModeManager_UpdateStopwatchLcd(void)
{
    stopWatch_t timeData;
    char timeText[11];
    uint32_t curTime = millis();

    if ((curTime - stopwatchLcdPrevUpdate) < STOPWATCH_LCD_UPDATE_PERIOD_MS) {
        return;
    }
    stopwatchLcdPrevUpdate = curTime;

    StopWatch_GetTime(&timeData);
    StopWatch_FormatTimeTenth(&timeData, timeText);
    ModeManager_PrintLine(1u, timeText);
}

static void ModeManager_EnterStopwatch(void)
{
    stopwatchLcdPrevUpdate = 0u;
    (void)lcd1602_i2c_clear();
    ModeManager_PrintLine(0u, "Stopwatch");
    ModeManager_UpdateStopwatchLcd();
    xil_printf("MODE Stopwatch\r\n");
}

static void ModeManager_EnterReserved(void)
{
    (void)lcd1602_i2c_clear();
    ModeManager_PrintLine(0u, "Reserved Mode");
    ModeManager_PrintLine(1u, "SW 000-100");
    xil_printf("MODE Reserved\r\n");
}

static void ModeManager_ChangeMode(SystemMode_t nextMode)
{
    if (modeInitialized != 0u && currentMode == nextMode) {
        return;
    }

    ModeManager_DisableLcdOwners();
    currentMode = nextMode;
    modeInitialized = 1u;

    switch (currentMode) {
    case MODE_STOPWATCH:
        ModeManager_EnterStopwatch();
        break;
    case MODE_CURRENT_TIME:
        LcdTime_Enable();
        break;
    case MODE_ATTENDANCE:
        Attendance_Enable();
        break;
    case MODE_DISTANCE:
        Distance_Enable();
        break;
    case MODE_TIME_SETTING:
        TimeSetting_Enable();
        break;
    default:
        ModeManager_EnterReserved();
        break;
    }
}

static void ModeManager_HandleTerminal(void)
{
    uint8_t data;

    while (UARTLITE_RxAvailable() != 0u) {
        data = UARTLITE_Receive();
        if (currentMode == MODE_TIME_SETTING) {
            TimeSetting_OnChar(data);
        }
        else if (currentMode == MODE_ATTENDANCE) {
            Attendance_Command(data);
        }
        else if (data == '?' || data == 'h' || data == 'H') {
            xil_printf("Modes: 000 Stopwatch, 001 Current Time, 010 RFID, 011 SR04, 100 Set Time\r\n");
        }
    }
}

static void ModeManager_HandleCustomUartButtons(void)
{
    if (Button_GetState(&hbtnLeft) == ACT_RELEASED) {
        UART_Transmit(UART0, 'r');
    }
    if (Button_GetState(&hbtnRight) == ACT_RELEASED) {
        UART_Transmit(UART0, 'c');
    }
}

void ModeManager_Init(void)
{
    ModeSwitch_Init();
    currentMode = MODE_RESERVED;
    modeInitialized = 0u;
    stopwatchLcdPrevUpdate = 0u;
}

void ModeManager_Excute(void)
{
    Clock_Excute();
    ModeManager_ChangeMode(ModeManager_ReadSwitchMode());
    ModeManager_HandleTerminal();

    switch (currentMode) {
    case MODE_STOPWATCH:
        StopWatch_Excute();
        ModeManager_HandleCustomUartButtons();
        ModeManager_UpdateStopwatchLcd();
        break;
    case MODE_CURRENT_TIME:
        LcdTime_Excute();
        break;
    case MODE_ATTENDANCE:
        Attendance_Excute();
        break;
    case MODE_DISTANCE:
        Distance_Excute();
        break;
    case MODE_TIME_SETTING:
        break;
    default:
        break;
    }
}

SystemMode_t ModeManager_GetMode(void)
{
    return currentMode;
}
