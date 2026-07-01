#ifndef SRC_DRIVER_FND_FND_H_
#define SRC_DRIVER_FND_FND_H_

#include "../../HAL/GPIO/GPIO.h"
#include <stdint.h>

#define FND_DATA_GPIO	GPIOA
#define FND_COM_GPIO	GPIOB

#define FND_DIGIT_0		0
#define FND_DIGIT_1		1
#define FND_DIGIT_2		2
#define FND_DIGIT_3		3

#define FND_DP_ON	 1
#define FND_DP_OFF 0

void FND_Init();
void FND_SetNum(uint32_t num);
void FND_Excute();
void FND_SelDigit(uint32_t digit);
void FND_DispDigit(uint32_t num, uint32_t fndDP);
void FND_DispAllOff();
void FND_DispNum(uint32_t num);
void FND_SetDP(uint32_t fndDigitSel, uint32_t fndDpState);


#endif /* SRC_DRIVER_FND_FND_H_ */
