#ifndef SRC_AP_MODEMANAGER_MODEMANAGER_H_
#define SRC_AP_MODEMANAGER_MODEMANAGER_H_

#include <stdint.h>

typedef enum {
    MODE_STOPWATCH = 0,
    MODE_CURRENT_TIME = 1,
    MODE_ATTENDANCE = 2,
    MODE_DISTANCE = 3,
    MODE_TIME_SETTING = 4,
    MODE_RESERVED = 7
} SystemMode_t;

void ModeManager_Init(void);
void ModeManager_Excute(void);
SystemMode_t ModeManager_GetMode(void);

#endif /* SRC_AP_MODEMANAGER_MODEMANAGER_H_ */
