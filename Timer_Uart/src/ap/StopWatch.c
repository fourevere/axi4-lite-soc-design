/*
 * StopWatch.c
 *
 *  Created on: 2026. 6. 24.
 *      Author: kccistc
 */

#include "StopWatch.h"
#include "../common/delay/delay.h"

stopWatch_e stopWatchState;
uint32_t stopWatchLed;
uint32_t stopWatchStateLed;
uint32_t counter;
stopWatch_t stopWatchTimeData;

volatile uint8_t rx_data;

void StopWatch_Init()
{
    LED_Init();
    FND_Init();
    Button_Init();

    stopWatchState = STOP;
    counter = 0;
    stopWatchLed = 0x01;
    stopWatchStateLed = 0;
    rx_data = 0;

    stopWatchTimeData.hour = 0;
    stopWatchTimeData.min = 0;
    stopWatchTimeData.sec = 0;
    stopWatchTimeData.ms = 0;
}

void StopWatch_Excute()
{
    StopWatch_RunTime();
    StopWatch_ControlState();
    StopWatch_DispWatch();
}

void StopWatch_DispWatch()
{
    FND_SetDP(FND_DIGIT_0, FND_DP_OFF);
    FND_SetDP(FND_DIGIT_1, FND_DP_ON);
    FND_SetDP(FND_DIGIT_2, FND_DP_OFF);
    FND_SetDP(FND_DIGIT_3, FND_DP_OFF);

    FND_SetNum((stopWatchTimeData.min % 10 * 1000) + (stopWatchTimeData.sec * 10) + (stopWatchTimeData.ms / 10));
    StopWatch_ControlLed();
}

void StopWatch_ControlState()
{
    switch (stopWatchState){
    case STOP:
        if(Button_GetState(&hbtnRunStop) == ACT_PUSHED){
            stopWatchState = RUN;
        }
        else if(Button_GetState(&hbtnClear) == ACT_PUSHED){
            stopWatchState = CLEAR;
        }
        else if(rx_data == 'r'){
            rx_data = 0;
            stopWatchState = RUN;
        }
        else if(rx_data == 'c'){
            rx_data = 0;
            stopWatchState = CLEAR;
        }
        break;
    case RUN:
        if(Button_GetState(&hbtnRunStop) == ACT_PUSHED){
            stopWatchState = STOP;
        }
        else if(rx_data == 'r'){
            rx_data = 0;
            stopWatchState = STOP;
        }
        break;
    case CLEAR:
        stopWatchState = STOP;
        counter = 0;
        StopWatch_ClearTime();
        break;
    default:
        stopWatchState = STOP;
        break;
    }
}

void StopWatch_ClearTime()
{
    stopWatchTimeData.hour = 0;
    stopWatchTimeData.min = 0;
    stopWatchTimeData.sec = 0;
    stopWatchTimeData.ms = 0;
}

void StopWatch_IncTime()
{
    if(stopWatchTimeData.ms == 99){
        stopWatchTimeData.ms = 0;
    }
    else {
        stopWatchTimeData.ms++;
        return;
    }
    if(stopWatchTimeData.sec == 59){
        stopWatchTimeData.sec = 0;
    }
    else {
        stopWatchTimeData.sec++;
        return;
    }
    if(stopWatchTimeData.min == 59){
        stopWatchTimeData.min = 0;
    }
    else {
        stopWatchTimeData.min++;
        return;
    }
    if(stopWatchTimeData.hour == 23){
        stopWatchTimeData.hour = 0;
    }
    else {
        stopWatchTimeData.hour++;
        return;
    }
}

void StopWatch_RunTime()
{
    static uint32_t prevTime = 0;
    uint32_t curTime = millis();

    if ((curTime - prevTime) < 10u) {
        return;
    }

    if (stopWatchState != RUN) {
        prevTime = curTime;
        return;
    }

    while ((curTime - prevTime) >= 10u) {
        prevTime += 10u;
        counter++;
        StopWatch_IncTime();
    }
}
void StopWatch_ControlLed()
{
    switch (stopWatchState){
    case STOP:
        StopWatch_StopLed();
        break;
    case RUN:
        StopWatch_RunLed();
        break;
    case CLEAR:
        StopWatch_ClearLed();
        break;
    default:
        break;
    }
}

void StopWatch_RunLed()
{
    static uint32_t prevTime = 0;
    uint32_t curTime = millis();

    stopWatchStateLed &= ~(1<< STOP_STATE_LED);
    stopWatchStateLed |= (1<< RUN_STATE_LED);
    LED_WritePort8(LED_HI_GPIO, stopWatchStateLed);

    if(curTime - prevTime < 100) return;
    prevTime = curTime;

    stopWatchLed = (stopWatchLed << 1) | (stopWatchLed >>7);
    LED_WritePort8(LED_LOW_GPIO, stopWatchLed);
}

void StopWatch_StopLed()
{
    stopWatchStateLed |= (1<< STOP_STATE_LED);
    stopWatchStateLed &= ~(1<< RUN_STATE_LED);
    LED_WritePort8(LED_HI_GPIO, stopWatchStateLed);
}

void StopWatch_ClearLed()
{
    stopWatchLed = 0x01;
    LED_WritePort8(LED_LOW_GPIO, stopWatchLed);
}

void StopWatch_GetTime(stopWatch_t *timeData)
{
    if (timeData == 0) {
        return;
    }
    *timeData = stopWatchTimeData;
}

void StopWatch_FormatTime(const stopWatch_t *timeData, char *buffer)
{
    if (timeData == 0 || buffer == 0) {
        return;
    }

    buffer[0] = (char)('0' + (timeData->hour / 10));
    buffer[1] = (char)('0' + (timeData->hour % 10));
    buffer[2] = ':';
    buffer[3] = (char)('0' + (timeData->min / 10));
    buffer[4] = (char)('0' + (timeData->min % 10));
    buffer[5] = ':';
    buffer[6] = (char)('0' + (timeData->sec / 10));
    buffer[7] = (char)('0' + (timeData->sec % 10));
    buffer[8] = '\0';
}


void StopWatch_FormatTimeTenth(const stopWatch_t *timeData, char *buffer)
{
    if (timeData == 0 || buffer == 0) {
        return;
    }

    StopWatch_FormatTime(timeData, buffer);
    buffer[8] = '.';
    buffer[9] = (char)('0' + (timeData->ms / 10));
    buffer[10] = '\0';
}
