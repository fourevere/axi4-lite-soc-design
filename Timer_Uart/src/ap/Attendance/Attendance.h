#ifndef SRC_AP_ATTENDANCE_ATTENDANCE_H_
#define SRC_AP_ATTENDANCE_ATTENDANCE_H_

#include <stdint.h>

void Attendance_Init(void);
void Attendance_Enable(void);
void Attendance_Disable(void);
void Attendance_Excute(void);
void Attendance_Command(uint8_t command);
uint8_t Attendance_IsRfidReady(void);

#endif /* SRC_AP_ATTENDANCE_ATTENDANCE_H_ */
