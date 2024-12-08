/*
 * methodsTest.c
 *
 *  Created on: Dec 5, 2024
 *      Author: mss3247
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
#include "math.h"
#include "PID.h"
#include "IMU.H"
#include <math.h>  // For fmodf and fabsf

#define PID_KP  1.2f
#define PID_KI  0.0f
#define PID_KD  0.1f
#define PID_TAU 0.00f
#define PID_LIM_MIN -180.0f
#define PID_LIM_MAX  180.0f
#define PID_Sampling 0.1f

///* Function to compute minimal angle difference [-180, 180] degrees */
//static float angle_difference(float target_angle, float current_angle)
//{
//    float diff = fmodf(target_angle - current_angle + 540.0f, 360.0f) - 180.0f;
//    return diff;
//}

void PIDController_Init(PIDController *pid)
{

    /* Clear controller variables */
    pid->integrator = 0.0f;
    pid->prevError = 0.0f;
    pid->differentiator = 0.0f;
    pid->prevMeasurement = 0.0f;
    pid->out = 0.0f;

}

float PIDController_Update(PIDController *pid, float setpoint,
                           float measurement)
{

    /*
     * Error signal (handling angle wrapping)
     */
    float error = angle_difference(setpoint, measurement);

    /*
     * Proportional
     */
    float proportional = pid->Kp * error;

    /*
     * Integral
     */
    pid->integrator += 0.5f * pid->Ki * pid->T * (error + pid->prevError);

    /* Anti-wind-up via integrator clamping */
    if (pid->integrator > pid->limMaxInt)
    {

        pid->integrator = pid->limMaxInt;

    }
    else if (pid->integrator < pid->limMinInt)
    {

        pid->integrator = pid->limMinInt;
    }

    /*
     * Derivative (band-limited differentiator)
     */
    float delta_measurement = angle_difference(measurement,
                                               pid->prevMeasurement);

    pid->differentiator = -(2.0f * pid->Kd * delta_measurement /* Note: derivative on measurement! */
    + (2.0f * pid->tau - pid->T) * pid->differentiator)
            / (2.0f * pid->tau + pid->T);

    /*
     * Compute output and apply limits
     */
    pid->out = proportional + pid->integrator + pid->differentiator;

    if (pid->out > pid->limMax)
    {
        pid->out = pid->limMax;
    }
    else if (pid->out < pid->limMin)
    {
        pid->out = pid->limMin;
    }

    /* Store error and measurement for later use */
    pid->prevError = error;
    pid->prevMeasurement = measurement;

    /* Return controller output */
    return pid->out;

}

/* Function to compute minimal angle difference [-180, 180] degrees */
static float angle_difference(float target_angle, float current_angle)
{
    float diff = (target_angle - current_angle);
    return diff;
}

//CHANGED TO SENSOR_DATA FOR NOW
void moveForwardDetect(oi_t *sensor_data, int totalDistance) // input total Distance in MM
{
    /*
     * Move forward as long as there isn't a low object or out of bounds tape | (bumpSensor == 0) && (cliffSensor == 0)
     * Also store the distance we've traveled and subtract that from the total distance then recursively call moveForwardDetect(sensor, newDistance)
     * If IR value is greater than 1800 or less than 1200 based on testing.
     * In testing FRONT sensors were the same. Right Sensor was ~1780 on floor. Left Sensor was ~2200 on floor. All sensors gave a reading of ~2700 for the tape
     */

    float distanceTraveled = 0;
    float remainingDistance = 0;

    // resets the distance for the oi
    int reset = sensor->distance;
    char distMoved[50];
    char buffer;
    int j;
    char note[10];





    oi_setWheels(0, 0);

    I2C1_Init();
    BNO055_Init();

    /* Initialise PID controller */
    PIDController pid = { PID_KP, PID_KI, PID_KD, PID_TAU, PID_LIM_MIN,
    PID_LIM_MAX };

    PIDController_Init(&pid);

    // Read and average initial heading
    float heading_sum = 0.0f;
    int num_readings = 10;
    int i;
    for (i = 0; i < num_readings; i++)
    {
        heading_sum += read_euler_heading(BNO055_ADDRESS_B) * (1.0f / 16.0f);
        timer_waitMillis(50);
    }
    float desired_heading = heading_sum / num_readings;

    // Initialize variables
    float current_heading = desired_heading;

    // Display initial headings
    printf("Initial Desired Heading: %.2f\n", desired_heading);
    printf("Initial Current Heading: %.2f\n", current_heading);

    float prev_error = 0.0f;
    float integral = 0.0f;

    // Define speed variables
    int16_t base_speed = 0;               // Starting from 0 speed
    const int16_t desired_base_speed = 75; // Target base speed (mm/s)
    const int16_t ramp_increment = 5;    // Speed increment per loop (mm/s)
    const int16_t max_wheel_speed = 300; // Max wheel speed per Roomba specs (mm/s)

    while ((distanceTraveled < totalDistance) && (sensor->bumpRight == 0)
            && (sensor->bumpLeft == 0)
            && ((sensor->cliffFrontLeftSignal < 2500)
                    && (sensor->cliffFrontLeftSignal > 200))
            && ((sensor->cliffFrontRightSignal < 2500)
                    && (sensor->cliffFrontRightSignal > 200))
            && ((sensor->cliffLeftSignal < 2500)
                    && (sensor->cliffLeftSignal > 200))
            && ((sensor->cliffRightSignal < 2500)
                    && (sensor->cliffRightSignal > 200)))

    {
        oi_update(sensor_data);

        // Read current heading

        float heading_sum = 0.0f;
        float num_readings = 10;
        int i;

        for (i = 0; i < num_readings; i++)
        {
            heading_sum += read_euler_heading(BNO055_ADDRESS_B)
                    * (1.0f / 16.0f);
            timer_waitMillis(50);
        }
        float current_heading = heading_sum / num_readings;

        // Compute heading error
        float error = angle_difference(desired_heading, current_heading);

        //PIDController_Update(PIDController *pid, desired_heading, current_heading);

        int16_t correct_angle_back;

        // Adjust wheel speeds
        int16_t right_speed = base_speed + (int16_t) correct_angle_back;
        int16_t left_speed = (base_speed - (int16_t) correct_angle_back);

        // Set wheel speeds
        oi_setWheels(right_speed, left_speed);

        // Accumulate distances
        totalDistance += sensor_data->distance;
        sensor_data->distance = 0;

        // Display debug information
        lcd_printf("Heading: %.2f\nError: %.2f\nDist: %d mm", current_heading,
                   error, total_distance);

        // Wait for next iteration
        timer_waitMillis((uint32_t) (PID_Sampling * 1000));

        // Handle stop conditions
        if (distance_since_last_stop >= stop_interval)
        {
            oi_setWheels(0, 0);
            base_speed = 0;  // Reset base speed for ramp-up
            timer_waitMillis(1000);
            distance_since_last_stop = 0;
        }

        // Ramp up speed if not at desired speed
        if (base_speed < desired_base_speed)
        {
            base_speed += 7;  // Increment by 5 mm/s
            if (base_speed > desired_base_speed)
            {
                base_speed = desired_base_speed;
            }
        }
    }
    //record the distance moved and send it to the GUI
    sprintf(distMoved, "moved\t%0.2f\n", distanceTraveled);
    cybot_send_string(distMoved);
    oi_setWheels(0, 0);
    remainingDistance = totalDistance - distanceTraveled;

    if (sensor->bumpLeft == 1)
    {
        cybot_send_string("bump left\n");
        Audio_Specified_Song(0x00, Small_Obj_Hit);
        moveBackward(sensor, 100);
        turnClockwise(sensor, 88.5); //CHANGE BASED ON TESTING FIXME  //88
        moveForwardDetect(sensor, 200); //CHANGE BASED ON TESTING
        turnCounterClockwise(sensor, 86.5); //CHANGE BASED ON TESTING  //88
        moveForwardDetect(sensor, (remainingDistance + 100));
    }

    else if (sensor->bumpRight == 1)
    {
        cybot_send_string("bump right\n");
        Audio_Specified_Song(0x00, Small_Obj_Hit);
        moveBackward(sensor, 100);
        turnCounterClockwise(sensor, 86.5); //88
        moveForwardDetect(sensor, 200); //CHANGE BASED ON TESTING FIXME
        turnClockwise(sensor, 88.5); //CHANGE BASED ON TESTING  //88
        moveForwardDetect(sensor, (remainingDistance + 100));
    }
    if (sensor->cliffLeftSignal >= 2500 || sensor->cliffLeftSignal <= 200
            || sensor->cliffFrontLeftSignal >= 2500
            || sensor->cliffFrontLeftSignal <= 200)
    {
        if (sensor->cliffLeftSignal >= 2500
                || sensor->cliffFrontLeftSignal >= 2500)
        {
            Audio_Specified_Song(0x00, Black_hole);
        }
        if (sensor->cliffLeftSignal >= 200
                || sensor->cliffFrontLeftSignal >= 200)
        {
            Audio_Specified_Song(0x00, wall);
        }
        cybot_send_string("Cliff Left Signal\n");
        buffer = uart_receive();
        j = 0;
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        moveBackward(sensor, 50); //CHANGE VALUE CHANGE PLACE
        lcd_printf("Note: %s", note);

        if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor, 88.5); //FIXME
            turnClockwise(sensor, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor, 86.5); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor, 88.5); //FIXME
        }
    }

    else if (sensor->cliffRightSignal >= 2500 || sensor->cliffRightSignal <= 200
            || sensor->cliffFrontRightSignal >= 2500
            || sensor->cliffFrontRightSignal <= 200)
    {
        if (sensor->cliffRightSignal >= 2500
                || sensor->cliffFrontRightSignal >= 2500)
        {
            Audio_Specified_Song(0x00, Black_hole);
        }
        if (sensor->cliffRightSignal >= 200
                || sensor->cliffFrontRightSignal >= 200)
        {
            Audio_Specified_Song(0x00, wall);
        }
        cybot_send_string("Cliff Right Signal\n");
        char buffer;
        int j;
        char note[10];
        buffer = uart_receive();
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        moveBackward(sensor, 50);
        lcd_printf("Note: %s", note);

        if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor, 88.5); //FIXME
            turnClockwise(sensor, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor, 86.5); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor, 88.5); //FIXME
        }
    }

}

