#ifndef SRC_AP_TIMESETTING_TIMESETTING_H_
#define SRC_AP_TIMESETTING_TIMESETTING_H_

#include <stdint.h>

void TimeSetting_Init(void);
void TimeSetting_Enable(void);
void TimeSetting_Disable(void);
void TimeSetting_OnChar(uint8_t data);

#endif /* SRC_AP_TIMESETTING_TIMESETTING_H_ */
