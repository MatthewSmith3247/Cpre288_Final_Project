//INCLUDE ALL THE INCLUDES
#include "adc.h" //YES
#include "uart.h" //YES
#include "Timer.h" //YES
#include "lcd.h" //YES
#include "math.h" //JUST MATH SO YES
#include "open_interface.h" //YES
#include "movement.h" //YES | NEED MOVEMENNT.C (NOT SURE ILL EVEN USE IT)
#include "button.h" //YES
#include "ping.h" //YES
#include "servo.h" //YES
#include "adc.h" //YES
#include "methods.h" //YES
#include "audio.h" //YES
#include "cyBot_uart.h"

int main(void)
{
    /* READ ME!
     * Here are a list of things i've altered so we can go back and change them.
     *  - uart.c and uart.h I changed it from "void uart_init(int baud)" to "void uart_init()"
     *  - In ping.c ping_read I commented out overflow_count. I think this was a variable used from Jessica's and Matt's lab.
     */

    //INITIALIZE EVERYTHING AND CHECK THE ORDER
    oi_t *sensorData = oi_alloc();
    oi_init(sensorData);

    timer_init();
    lcd_init();
    button_init();
    cyBot_uart_init();

    //adc_init();
    //uart_init();
    //ping_init();
    //servo_init();

    /* I want to Detect (move forward as long as there isn't an low object or out of bounds tape).
     * I also want to have a method called fastScan (scan every X amount have a new .c called methods that will contain all of our methods such as moveForwardount of degrees) that will be much faster than a normal full scan.
     * I also want a method objectCollision that will return true or false that will calculate if an object will hit the cyBot based on the angle and distance from the cyBot, using the fastScan data.
     * The cyBot will continue to move forward on its desired path from A to B, doing a fast scan every X (half meter?) distance as long as the objectCollision returns false.
     * If the object collision determines that an object will hit the cyBot, we will perform a fullScan that will scan every 2 degrees.
     * We will then handle moving around the object and getting back to its desired path. We must store how far we have currently traveled and subtract that from the total distance we need to travel.
     * While avoiding the tall object we will use the moveForwardDetect for all movement so we do not run though a low object while avoiding a tall object.
     * Could include a method that will only scan from 45 to 135 to see if the cybot will hit something instead of useing a faster scan for 180 degrees.
     */

    while(1)
    {
       printIR(sensorData);
    }

    moveForwardDetect(sensorData, 2000);

    return 0;
}
