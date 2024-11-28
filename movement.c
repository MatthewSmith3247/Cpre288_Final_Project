#include "open_interface.h"
#include "uart.h"


void turnCounterClockwise(oi_t *sensor, int degrees)
{
double sum = 0;
oi_setWheels(50, -50); //right wheel | left wheel
while (sum < degrees)
{

    oi_update(sensor);
    sum += sensor->angle;
}
cybot_send_string("counterclockwise\n");
oi_setWheels(0, 0); //stop
}

void turnClockwise(oi_t *sensor, int degrees)
{
double sum = 0;
oi_setWheels(-50, 50); //right wheel | left wheel
while (sum < degrees)
{

    oi_update(sensor);
    sum -= sensor->angle;
}
cybot_send_string("clockwise\n");
oi_setWheels(0, 0); //stop
}

void moveForward(oi_t *sensor, int centimeters) {
    double sum = 0;
    oi_setWheels(50, 50);
    char sendData[50];
    while (sum < centimeters)
    {
        oi_update(sensor);
        sum += sensor->distance;
    }
    sprintf(sendData, "moved\t%d\n", centimeters);
    cybot_send_string(sendData);
    oi_setWheels(0, 0); // stop
}

void moveBackward(oi_t *sensor, int centimeters) {
    double sum = 0;
    oi_setWheels(-50, -50);
    while (sum < centimeters)
    {
        oi_update(sensor);
        sum -= sensor->distance;
    }

    oi_setWheels(0, 0); // stop
}



