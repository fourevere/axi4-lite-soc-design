#include "xil_printf.h"
#include "ap/Attendance/Attendance.h"
#include "ap/Clock/Clock.h"
#include "ap/Distance/Distance.h"
#include "ap/LcdTime/LcdTime.h"
#include "ap/ModeManager/ModeManager.h"
#include "ap/StopWatch.h"
#include "ap/TimeSetting/TimeSetting.h"
#include "common/interrupt/interrupt.h"
#include "HAL/I2C/I2C.h"
#include "HAL/TMR/TMR.h"
#include "HAL/UART/UART.h"

int main()
{
    StopWatch_Init();
    Clock_Init();

    TMR_SetPSC(TMR0, 100 - 1);
    TMR_SetARR(TMR0, 1000 - 1);

    I2C_Init(I2C0);

    if (SetupInterruptSystem() != XST_SUCCESS) {
        xil_printf("Interrupt setup failed\r\n");
        return -1;
    }

    UART_StartInterrupt(UART0);
    TMR_StartInterrupt(TMR0);
    TMR_StartTimer(TMR0);

    LcdTime_Init();
    Attendance_Init();
    Distance_Init();
    TimeSetting_Init();
    ModeManager_Init();

    xil_printf("SR04 LCD RFID system ready\r\n");

    while (1) {
        ModeManager_Excute();
    }

    return 0;
}
