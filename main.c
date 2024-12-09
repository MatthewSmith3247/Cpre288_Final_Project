#include "adc.h" 
#include "uart.h" 
#include "movement.h" 
#include "button.h" 
#include "ping.h" 
#include "servo.h" 
#include "adc.h" 
#include "methods.h" 
#include "audio.h" 
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
#include "math.h"
#include "IMU.H"
#include "manual.h"





int main(void)
{
    oi_t *sensor = oi_alloc();
    oi_init(sensor);

    oi_setWheels(0,0);


    timer_init();
    lcd_init();
    //button_init();
    configure_timer();
    //init_button_interrupts();
    adc_init();
    uart_init(115200);
    servo_init();
    Audio_Init();
    //I2C1_Init(); //now tied into all scans
    //BNO055_Init(); //now tied into all scans
    connectToGui();
    //autoManualDriver(sensor);
    manualDriver(sensor);


// used for testing scan data accuracy.
//while (1){
//    fastScan(0, 180);
//    objectAvoid(sensor);
//}



return 0;
}
