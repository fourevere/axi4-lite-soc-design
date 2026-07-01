#include "LED.h"

void LED_Init() {
	GPIO_SetMode(LED_LOW_GPIO, 0xff);
	GPIO_SetMode(LED_HI_GPIO, 0xff);
}

void LED_WritePort8(GPIO_TypeDef *LedGPIOx, uint16_t led) {
	GPIO_WritePort(LedGPIOx, led);
}

void LED_WritePort16(uint16_t led) {
	uint32_t ledTemp;

	ledTemp = led & 0x00ff;
	GPIO_WritePort(LED_LOW_GPIO, ledTemp);
	ledTemp = (led>>8) & 0x00ff;
	GPIO_WritePort(LED_HI_GPIO, ledTemp);
}

void LED_PinOn(uint16_t ledPin) {
	uint16_t ledPinTemp;
	uint32_t ledPortState;

	ledPinTemp = 1 << ledPin;

	ledPortState = GPIO_GetODR(LED_LOW_GPIO);
	ledPortState |= (ledPinTemp & 0x00ff);
	GPIO_WritePort(LED_LOW_GPIO, ledPortState);

	ledPortState = GPIO_GetODR(LED_HI_GPIO);
	ledPortState |= ((ledPinTemp >> 8) & 0x00ff);
	GPIO_WritePort(LED_HI_GPIO, ledPortState);
}

void LED_PinOff(uint16_t ledPin) {
	uint16_t ledPinTemp;
	uint32_t ledPortState;

	ledPinTemp = 1 << ledPin;

	ledPortState = GPIO_GetODR(LED_LOW_GPIO);
	ledPortState &= ~(ledPinTemp & 0x00ff);
	GPIO_WritePort(LED_LOW_GPIO, ledPortState);

	ledPortState = GPIO_GetODR(LED_HI_GPIO);
	ledPortState &= ~((ledPinTemp >> 8) & 0x00ff);
	GPIO_WritePort(LED_HI_GPIO, ledPortState);
}

void LED_PinToggle(uint16_t ledPin) {
	uint16_t ledPinTemp;
	uint32_t ledPortState;

	ledPinTemp = 1 << ledPin;

	ledPortState = GPIO_GetODR(LED_LOW_GPIO);
	ledPortState ^= (ledPinTemp & 0x00ff);
	GPIO_WritePort(LED_LOW_GPIO, ledPortState);

	ledPortState = GPIO_GetODR(LED_HI_GPIO);
	ledPortState ^= ((ledPinTemp >> 8) & 0x00ff);
	GPIO_WritePort(LED_HI_GPIO, ledPortState);
}
