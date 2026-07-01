#include "Clock.h"
#include "../../common/delay/delay.h"

#define CLOCK_UPDATE_PERIOD_MS 100u

static ClockTime_t clockTimeData;
static uint32_t clockPrevTime;

static void Clock_IncTenth(void)
{
    if (clockTimeData.tenth == 9u) {
        clockTimeData.tenth = 0u;
    }
    else {
        clockTimeData.tenth++;
        return;
    }

    if (clockTimeData.sec == 59u) {
        clockTimeData.sec = 0u;
    }
    else {
        clockTimeData.sec++;
        return;
    }

    if (clockTimeData.min == 59u) {
        clockTimeData.min = 0u;
    }
    else {
        clockTimeData.min++;
        return;
    }

    if (clockTimeData.hour == 23u) {
        clockTimeData.hour = 0u;
    }
    else {
        clockTimeData.hour++;
    }
}

void Clock_Init(void)
{
    clockTimeData.hour = 0u;
    clockTimeData.min = 0u;
    clockTimeData.sec = 0u;
    clockTimeData.tenth = 0u;
    clockPrevTime = millis();
}

void Clock_Excute(void)
{
    uint32_t curTime = millis();

    while ((curTime - clockPrevTime) >= CLOCK_UPDATE_PERIOD_MS) {
        clockPrevTime += CLOCK_UPDATE_PERIOD_MS;
        Clock_IncTenth();
    }
}

void Clock_SetTime(uint8_t hour, uint8_t min, uint8_t sec)
{
    if (hour > 23u || min > 59u || sec > 59u) {
        return;
    }

    clockTimeData.hour = hour;
    clockTimeData.min = min;
    clockTimeData.sec = sec;
    clockTimeData.tenth = 0u;
    clockPrevTime = millis();
}

void Clock_GetTime(ClockTime_t *timeData)
{
    if (timeData == 0) {
        return;
    }
    *timeData = clockTimeData;
}

void Clock_FormatTime(const ClockTime_t *timeData, char *buffer)
{
    if (timeData == 0 || buffer == 0) {
        return;
    }

    buffer[0] = (char)('0' + (timeData->hour / 10u));
    buffer[1] = (char)('0' + (timeData->hour % 10u));
    buffer[2] = ':';
    buffer[3] = (char)('0' + (timeData->min / 10u));
    buffer[4] = (char)('0' + (timeData->min % 10u));
    buffer[5] = ':';
    buffer[6] = (char)('0' + (timeData->sec / 10u));
    buffer[7] = (char)('0' + (timeData->sec % 10u));
    buffer[8] = '\0';
}

void Clock_FormatTimeTenth(const ClockTime_t *timeData, char *buffer)
{
    if (timeData == 0 || buffer == 0) {
        return;
    }

    Clock_FormatTime(timeData, buffer);
    buffer[8] = '.';
    buffer[9] = (char)('0' + timeData->tenth);
    buffer[10] = '\0';
}
