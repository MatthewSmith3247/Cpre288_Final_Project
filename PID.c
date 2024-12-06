/*
 * PID.c
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

void PID_GO_STRGHT(void) //NEEDS TO HAVE IMU LIBRARY AND HAVE THE
{

#define PID_KP  1.2f
#define PID_KI  0.0f
#define PID_KD  0.1f
#define PID_TAU 0.00f

#define PID_LIM_MIN -180.0f
#define PID_LIM_MAX  180.0f

#define PID_Sampling 0.1f

    lcd_init();
    timer_init();

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
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

    int total_distance = 0;            // Total distance traveled in mm
    int distance_since_last_stop = 0;  // Distance since last stop in mm
    const int stop_interval = 600;     // 60 cm in mm
    const int max_total_distance = 3960; // 3.96 m in mm

    // Define speed variables
    int16_t base_speed = 0;               // Starting from 0 speed
    const int16_t desired_base_speed = 75; // Target base speed (mm/s)
    const int16_t ramp_increment = 5;    // Speed increment per loop (mm/s)
    const int16_t max_wheel_speed = 300; // Max wheel speed per Roomba specs (mm/s)

    while (total_distance < max_total_distance)
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
        total_distance += sensor_data->distance;
        distance_since_last_stop += sensor_data->distance;
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

    // Stop the robot
    oi_setWheels(0, 0);

    // Free resources
    oi_free(sensor_data);

}

