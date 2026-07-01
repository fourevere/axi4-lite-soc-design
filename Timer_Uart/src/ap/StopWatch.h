/*
 * StopWatch.h
 *
 *  Created on: 2026. 6. 24.
 *      Author: kccistc
 */

#ifndef SRC_AP_STOPWATCH_H_
#define SRC_AP_STOPWATCH_H_
#include <stdint.h>
#include "../driver/Button/Button.h"
#include "../driver/FND/FND.h"
#include "../driver/LED/LED.h"

typedef enum{
    STOP = 0,
    RUN,
    CLEAR
}stopWatch_e;

typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t ms;
} stopWatch_t;

#define STOP_STATE_LED 5
#define RUN_STATE_LED  7

void StopWatch_Init();
void StopWatch_Excute();
void StopWatch_DispWatch();
void StopWatch_ControlState();
void StopWatch_ClearTime();
void StopWatch_IncTime();
void StopWatch_RunTime();
void StopWatch_ControlLed();
void StopWatch_RunLed();
void StopWatch_StopLed();
void StopWatch_ClearLed();
void StopWatch_GetTime(stopWatch_t *timeData);
void StopWatch_FormatTime(const stopWatch_t *timeData, char *buffer);
void StopWatch_FormatTimeTenth(const stopWatch_t *timeData, char *buffer);

#endif /* SRC_AP_STOPWATCH_H_ */
