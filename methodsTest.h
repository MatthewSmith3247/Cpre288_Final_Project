/*
 * methodsTest.h
 *
 *  Created on: Dec 5, 2024
 *      Author: mss3247
 */

#ifndef METHODSTEST_H_
#define METHODSTEST_H_

typedef struct
{

    /* Controller gains */
    float Kp;
    float Ki;
    float Kd;

    /* Derivative low-pass filter time constant */
    float tau;

    /* Output limits */
    float limMin;
    float limMax;

    /* Integrator limits */
    float limMinInt;
    float limMaxInt;

    /* Sample time (in seconds) */
    float T;

    /* Controller "memory" */
    float integrator;
    float prevError; /* Required for integrator */
    float differentiator;
    float prevMeasurement; /* Required for differentiator */

    /* Controller output */
    float out;

}PIDController;

void PIDController_Init(PIDController *pid);
float PIDController_Update(PIDController *pid, float setpoint,
                           float measurement);
static float angle_difference(float target_angle, float current_angle);
void moveForwardDetect(oi_t *sensor, int totalDistance);

#endif /* METHODSTEST_H_ */
