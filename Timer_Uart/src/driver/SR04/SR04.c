#include "SR04.h"
#include "../../common/delay/delay.h"
#include "../../HAL/TMR/TMR.h"

static uint32_t SR04_Micros(void)
{
    uint32_t ms1;
    uint32_t ms2;
    uint32_t cnt;

    do {
        ms1 = millis();
        cnt = TMR_GetCNT(TMR0);
        ms2 = millis();
    } while (ms1 != ms2);

    return (ms1 * 1000u) + cnt;
}

void SR04_Init(void)
{
    uint32_t mode = GPIO_GetCR(SR04_GPIO);

    mode &= ~SR04_ECHO_PIN;
    mode |= SR04_TRIG_PIN;
    GPIO_SetMode(SR04_GPIO, mode);
    GPIO_WritePin(SR04_GPIO, SR04_TRIG_PIN, GPIO_RESET);
}

uint8_t SR04_ReadEchoPin(void)
{
    return (uint8_t)GPIO_ReadPin(SR04_GPIO, SR04_ECHO_PIN);
}

uint32_t SR04_ReadEchoTimeUs(void)
{
    uint32_t timeoutStart;
    uint32_t echoStart;
    uint32_t echoEnd;

    GPIO_WritePin(SR04_GPIO, SR04_TRIG_PIN, GPIO_RESET);
    delay_us(2u);

    GPIO_WritePin(SR04_GPIO, SR04_TRIG_PIN, GPIO_SET);
    delay_us(SR04_TRIGGER_PULSE_US);
    GPIO_WritePin(SR04_GPIO, SR04_TRIG_PIN, GPIO_RESET);

    timeoutStart = SR04_Micros();
    while (GPIO_ReadPin(SR04_GPIO, SR04_ECHO_PIN) == 0u) {
        if ((SR04_Micros() - timeoutStart) > SR04_TIMEOUT_US) {
            return 0u;
        }
    }

    echoStart = SR04_Micros();
    while (GPIO_ReadPin(SR04_GPIO, SR04_ECHO_PIN) != 0u) {
        if ((SR04_Micros() - echoStart) > SR04_TIMEOUT_US) {
            return 0u;
        }
    }

    echoEnd = SR04_Micros();
    return echoEnd - echoStart;
}

uint32_t SR04_ReadDistanceMm(void)
{
    uint32_t echo = SR04_ReadEchoTimeUs();

    if (echo == 0u) {
        return 0u;
    }

    return (echo * 10u) / 58u;
}

uint32_t SR04_ReadDistanceMmFiltered(void)
{
    uint32_t data[5];
    uint32_t count = 0u;
    uint32_t i;
    uint32_t j;
    uint32_t temp;

    for (i = 0u; i < 5u; i++) {
        uint32_t distance = SR04_ReadDistanceMm();

        if (distance != 0u) {
            data[count] = distance;
            count++;
        }

        delay_ms(60u);
    }

    if (count == 0u) {
        return 0u;
    }

    for (i = 0u; i < count - 1u; i++) {
        for (j = i + 1u; j < count; j++) {
            if (data[i] > data[j]) {
                temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
    }

    return data[count / 2u];
}
