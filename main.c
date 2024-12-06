#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
#include "math.h"
#include "IMU.H"
#include "methodsTest.h"

int main(void)
{
    oi_t *sensorData = oi_alloc();
    oi_init(sensorData);

    lcd_init();
    timer_init();

    I2C1_Init();
    BNO055_Init();

    moveForwardDetect(sensorData, 500);
    timer_waitMillis(2000);
    moveForwardDetect(sensorData, 500);
    timer_waitMillis(2000);
    moveForwardDetect(sensorData, 500);
    timer_waitMillis(2000);

}
