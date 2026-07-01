/*
 * Button.c
 *
 *  Created on: 2026. 6. 24.
 *      Author: kccistc
 */

#include "Button.h"

hbutton hbtnRunStop, hbtnClear;
hbutton hbtnLeft, hbtnRight;

void Button_Init(){
	uint32_t btnPort = GPIO_GetCR(GPIOB);
	btnPort &= ~(1<<4 | 1<<5 | 1<<6 | 1<<7);
	GPIO_SetMode(GPIOB, btnPort);

	Button_SetInit(&hbtnRunStop, GPIOB, GPIO_PIN_4);
	Button_SetInit(&hbtnClear, GPIOB, GPIO_PIN_5);
	Button_SetInit(&hbtnLeft, GPIOB, GPIO_PIN_6);
	Button_SetInit(&hbtnRight, GPIOB, GPIO_PIN_7);
}


void Button_SetInit(hbutton *hbtn, GPIO_TypeDef *GPIOx, uint32_t gpio_pin) {
	hbtn->GPIOx = GPIOx;
	hbtn->gpio_pin = gpio_pin;
	hbtn->prevState = RELEASED;
}

button_status_e Button_GetState(hbutton *hbtn){
	button_state_e curState = GPIO_ReadPin(hbtn->GPIOx, hbtn->gpio_pin) ? PUSHED : RELEASED;

	if(hbtn->prevState == RELEASED && curState == PUSHED) {
		hbtn->prevState = curState;
		delay_ms(5);
		return ACT_PUSHED;
	}
	else if (hbtn->prevState == PUSHED && curState == RELEASED) {
		hbtn->prevState = curState;
		delay_ms(5);
		return ACT_RELEASED;
	}
	return NO_ACT;
}
