#ifndef SRC_DRIVER_SR04_SR04_H_
#define SRC_DRIVER_SR04_SR04_H_

#include <stdint.h>
#include "../../HAL/GPIO/GPIO.h"

#define SR04_GPIO       GPIOE
#define SR04_TRIG_PIN   GPIO_PIN_3
#define SR04_ECHO_PIN   GPIO_PIN_4
#define SR04_TIMEOUT_US 30000u
#define SR04_TRIGGER_PULSE_US 10u

void SR04_Init(void);
uint8_t SR04_ReadEchoPin(void);
uint32_t SR04_ReadEchoTimeUs(void);
uint32_t SR04_ReadDistanceMm(void);
uint32_t SR04_ReadDistanceMmFiltered(void);

#endif /* SRC_DRIVER_SR04_SR04_H_ */
