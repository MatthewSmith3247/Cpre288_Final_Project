#include "open_interface.h"
#include "uart.h"
#include "movement.h"

//#define DEGREE_ACCURACY 0.5
//#define MM_ACCURACY 1
// Wheel powers specifically calibrated for Cybot 2
short forward_RWP = 95; // Right wheel power for forward movement
short forward_LWP = 80; // Left wheel power for forward movement
short turn_RWP = 50;    // Right wheel power for turning
short turn_LWP = 50;    // Left wheel power for turning


void turnCounterClockwise(oi_t *sensor, float degrees)
{
double sum = 0;
oi_setWheels(50, -50); //right wheel | left wheel
while (sum < degrees)
{

    oi_update(sensor);
    sum += sensor->angle;
}
oi_setWheels(0, 0); //stop
cybot_send_string("counterclockwise\n");

}

void turnClockwise(oi_t *sensor, float degrees)
{
double sum = 0;
oi_setWheels(-50, 50); //right wheel | left wheel
while (sum < degrees)
{

    oi_update(sensor);
    sum -= sensor->angle;
}
oi_setWheels(0, 0); //stop
cybot_send_string("clockwise\n");

}


//**
// * Move the robot forward by a specified distance in cm.
// * Dynamically adjusts wheel power to ensure straight movement.
// */
void moveForward(oi_t *sensor, float distance_cm) {
    float total_distance = 0; // Distance traveled in mm
    float angle_correction = 0;
    char sendData[50];


    while (total_distance < (distance_cm)) { // Convert cm to mm
        oi_update(sensor);

        total_distance += sensor->distance; // Accumulate distance
        angle_correction += sensor->angle; // Accumulate angle deviation

        // Adjust wheel power based on angle deviation
        if (angle_correction > DEGREE_ACCURACY) {
            forward_RWP -= 1; // Reduce right wheel power
            angle_correction = 0;
        } else if (angle_correction < -DEGREE_ACCURACY) {
            forward_RWP += 1; // Increase right wheel power
            angle_correction = 0;
        }

        oi_setWheels(forward_RWP, forward_LWP); // Set adjusted wheel speeds
    }

    oi_setWheels(0, 0); // Stop the robot
    sprintf(sendData, "moved\t%.0f\n", total_distance);
    cybot_send_string(sendData);
//    return total_distance; // Return traveled distance in cm
}


void moveBackward(oi_t *sensor, float distance_cm) {
   char  sendData[50];

    float total_distance = 0; // Distance traveled in mm
    float angle_correction = 0;

    while (total_distance > -(distance_cm)) { // Convert cm to mm
        oi_update(sensor);

        total_distance += sensor->distance; // Accumulate distance
        angle_correction += sensor->angle; // Accumulate angle deviation

        // Adjust wheel power based on angle deviation
        if (angle_correction > DEGREE_ACCURACY) {
            forward_RWP += 1; // Increase right wheel magnitude
            angle_correction = 0;
        } else if (angle_correction < -DEGREE_ACCURACY) {
            forward_RWP -= 1; // Decrease right wheel magnitude
            angle_correction = 0;
        }

        oi_setWheels(-forward_RWP, -forward_LWP); // Reverse wheel speeds
    }

    oi_setWheels(0, 0); // Stop the robot
    sprintf(sendData, "moved\t%.0f\n", -1 * (distance_cm)); //POSSIBLE CHANGE
    cybot_send_string(sendData);
//    return total_distance / 10.0; // Return traveled distance in cm
}




