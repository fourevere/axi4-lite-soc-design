/*
 * delay.c
 *
 *  Created on: 2026. 6. 24.
 *      Author: kccistc
 */

#include "delay.h"

uint32_t m_tick = 0;

uint32_t millis() {
	return m_tick;
}

void incTick() {
	m_tick++;
}

void delay_sec(uint32_t useconds) {
	sleep(useconds);
}

void delay_ms(uint32_t msec) {
	uint32_t mseconds = msec * 1000;
	usleep(mseconds);
}

void delay_us(uint32_t usec) {
	usleep(usec);
}

