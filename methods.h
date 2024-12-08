#ifndef METHODS_H_
#define METHODS_H_

////void cybotSendString(char string[50]);
//typedef struct
//{
//
//    /* Controller gains */
//    float Kp;
//    float Ki;
//    float Kd;
//
//    /* Derivative low-pass filter time constant */
//    float tau;
//
//    /* Output limits */
//    float limMin;
//    float limMax;
//
//    /* Integrator limits */
//    float limMinInt;
//    float limMaxInt;
//
//    /* Sample time (in seconds) */
//    float T;
//
//    /* Controller "memory" */
//    float integrator;
//    float prevError; /* Required for integrator */
//    float differentiator;
//    float prevMeasurement; /* Required for differentiator */
//
//    /* Controller output */
//    float out;
//
//} PIDController;
//
//int turn_degrees(oi_t *sensor_data, float degrees);
//
//void PIDController_Init(PIDController *pid);
//float PIDController_Update(PIDController *pid, float setpoint,
//                           float measurement);
//float angle_difference(float target_angle, float current_angle);
void moveForwardDetect(oi_t *sensor, int totalDistance);
void moveForwardDetect(oi_t *sensor, int centimeters);
int objectCollision();
int fastScan(int startDeg, int endDeg);
int fullScan(int startDeg, int endDeg);
void printIR(oi_t *sensor);
void sendToPutty(char string[], int length);
float calculate_average_heading(void);
void objectAvoid(oi_t *sensor);
void autoManualDriver(oi_t *sensor_data);
void find_bathroom(oi_t *sensor_data);
void find_kitchen(oi_t *sensor_data);
void find_livingRoom(oi_t *sensor_data);
void find_exit(oi_t *sensor_data);
//float PID_linear_movement(oi_t * sensor, int totalDistance);
//void bubble_sort(float arr[], int n);

void oneObject(oi_t *sensor);
void twoObjects(oi_t *sensor);
void moreObjects(oi_t *sensor);

#endif
