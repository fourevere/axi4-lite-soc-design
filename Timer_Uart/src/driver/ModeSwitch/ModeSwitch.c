#include "ModeSwitch.h"

void ModeSwitch_Init(void)
{
    uint32_t mode = GPIO_GetCR(MODE_SWITCH_GPIO);

    mode &= ~MODE_SWITCH_MASK;
    GPIO_SetMode(MODE_SWITCH_GPIO, mode);
}

uint8_t ModeSwitch_Read(void)
{
    return (uint8_t)(GPIO_ReadPort(MODE_SWITCH_GPIO) & MODE_SWITCH_MASK);
}
