#ifndef SRC_AP_CLOCK_CLOCK_H_
#define SRC_AP_CLOCK_CLOCK_H_

#include <stdint.h>

typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t tenth;
} ClockTime_t;

void Clock_Init(void);
void Clock_Excute(void);
void Clock_SetTime(uint8_t hour, uint8_t min, uint8_t sec);
void Clock_GetTime(ClockTime_t *timeData);
void Clock_FormatTime(const ClockTime_t *timeData, char *buffer);
void Clock_FormatTimeTenth(const ClockTime_t *timeData, char *buffer);

#endif /* SRC_AP_CLOCK_CLOCK_H_ */
