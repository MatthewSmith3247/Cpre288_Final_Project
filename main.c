//INCLUDE ALL THE INCLUDES
#include "adc.h" //YES
#include "uart.h" //YES
#include "movement.h" //YES | NEED MOVEMENNT.C (NOT SURE ILL EVEN USE IT)
#include "button.h" //YES
#include "ping.h" //YES
#include "servo.h" //YES
#include "adc.h" //YES
#include "methods.h" //YES
#include "audio.h" //YES
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
    //I2C1_Init(); //now tied into all scans -- DO NOT COMMENT OUT
    //BNO055_Init(); //now tied into all scans -- DO NOT COMMENT OUT
    connectToGui();
    //autoManualDriver(sensor);
    manualDriver(sensor);



//while (1){
//    fastScan(0, 180);
//    objectAvoid(sensor);
//}



return 0;
}
