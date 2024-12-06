/*
 * test_filename.h
 *
 *  Created on: Dec 3, 2024
 *      Author: mss3247
 */

#ifndef TEST_FILENAME_H_
#define TEST_FILENAME_H_


/*
 * test_file.c
 *
 *  Created on: Dec 3, 2024
 *      Author: mss3247
 */

/*
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"


#define FILTER_SIZE 10
#define ALPHA 0.1  // Smoothing factor for low-pass filter


// Function Prototypes
void I2C1_Init(void);
int I2C1_Write(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
int I2C1_Read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);


//Linear Acceleration Readings
int16_t read_linear_acceleration_x(uint8_t device_addr);
int16_t read_linear_acceleration_y(uint8_t device_addr);


//Filtering Data Readings
int16_t get_moving_average(int16_t *buffer);
void update_moving_average(int16_t *buffer, int16_t new_value);
int16_t apply_low_pass_filter(int16_t current_value, int16_t *prev_filtered);
int16_t combine_filters(int16_t raw_value, int16_t *buffer, int16_t *prev_filtered);

// Buffers for moving average
int16_t x_axis_buffer[FILTER_SIZE] = {0};
int16_t y_axis_buffer[FILTER_SIZE] = {0};
int index = 0;

// Previous output for low-pass filter
int16_t x_axis_prev_filtered = 0;
int16_t y_axis_prev_filtered = 0;


// Scaling Raw Value FIXME
float scale_acceleration(int16_t raw_value);
float read_linear_acceleration_x_scaled(uint8_t device_addr);
float read_linear_acceleration_y_scaled(uint8_t device_addr);

int main(void){


    lcd_init();
    timer_init();


    uint8_t data_write = 0x0C;
    uint8_t data_read;
    uint8_t data_read2;
    int count = 0;

    I2C1_Init();
    //Configure to be in NDOF Mode
    I2C1_Write(0x29,0x3D,&data_write,1);

    //Read the register to see if Calibrated
    I2C1_Read(0x29, 0x35, &data_read, 1);
    I2C1_Read(0x29, 0x36, &data_read2, 1);


    int16_t x_axis_raw = 0, y_axis_raw = 0;
    int16_t x_axis_filtered = 0, y_axis_filtered = 0;


    float x_axis_scaled = 0, y_axis_scaled = 0;

    while (1) {
        // Read and scale linear acceleration
        x_axis_scaled = read_linear_acceleration_x_scaled(0x29);
        y_axis_scaled = read_linear_acceleration_y_scaled(0x29);

        // Display scaled values
        lcd_printf("Lin Accel (m/s^2):\n- X: %.2f\n- Y: %.2f\n - Count: %d", x_axis_scaled, y_axis_scaled, count);
         count++;
        // Delay
        timer_waitMillis(500);
    }

    return 0;
}


/////////////////////////////////////////////Functions


// Initialize I2C1 for communication
void I2C1_Init(void) {
    // Enable the clock for I2C1 and Port A
    SYSCTL_RCGCI2C_R |= 0x02;  // Enable I2C1 clock
    SYSCTL_RCGCGPIO_R |= 0x01;  // Enable GPIO Port A clock

    while ((SYSCTL_PRI2C_R & 0x02) == 0) {};  // Wait for I2C1 clock to stabilize
    while ((SYSCTL_PRGPIO_R & 0x01) == 0) {}; // Wait for Port A clock to stabilize

    // Configure PA6 (SCL) and PA7 (SDA) for I2C
    GPIO_PORTA_AFSEL_R |= 0xC0;  // Enable alternate functions on PA6, PA7
    GPIO_PORTA_ODR_R |= 0x80;    // Enable open drain on PA7 (SDA)
    GPIO_PORTA_DEN_R |= 0xC0;    // Enable digital functions on PA6, PA7
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0x00FFFFFF) | 0x33000000; // Assign I2C functions

    // Initialize I2C1 Master
    I2C1_MCR_R = 0x10;  // Set I2C Master mode
    I2C1_MTPR_R = 7;    // Set SCL clock speed to ~100kHz (assuming 16 MHz system clock)
}


// Write data to a specific register on the BNO055
int I2C1_Write(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
    I2C1_MSA_R = (device_addr << 1) & ~0x01;  // Set slave address and write mode

    I2C1_MDR_R = reg_addr;  // Write the register address
    I2C1_MCS_R = 0x03;      // Start and Run
    while (I2C1_MCS_R & 0x01) {};  // Wait for completion

    if (I2C1_MCS_R & 0x02) return -1;  // Error occurred

    // Write data bytes
         uint8_t i;
    for (i = 0; i < len; i++) {
        I2C1_MDR_R = data[i];  // Write the next byte
        I2C1_MCS_R = (i == (len - 1)) ? 0x05 : 0x01;  // Stop for the last byte
        while (I2C1_MCS_R & 0x01) {};  // Wait for completion
        if (I2C1_MCS_R & 0x02) return -1;  // Error occurred
    }

    return 0;  // Success
}

// Read data from a specific register on the BNO055
int I2C1_Read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
    // Send register address
    I2C1_MSA_R = (device_addr << 1) & ~0x01;  // Set slave address and write mode
    I2C1_MDR_R = reg_addr;  // Write the register address
    I2C1_MCS_R = 0x03;      // Start and Run
    while (I2C1_MCS_R & 0x01) {};  // Wait for completion
    if (I2C1_MCS_R & 0x02) return -1;  // Error occurred

    // Read data bytes
    I2C1_MSA_R = (device_addr << 1) | 0x01;  // Set slave address and read mode
        uint8_t i;
    for (i = 0; i < len; i++) {
        I2C1_MCS_R = (i == (len - 1)) ? 0x07 : 0x09;  // Stop for the last byte
        while (I2C1_MCS_R & 0x01) {};  // Wait for completion
        if (I2C1_MCS_R & 0x02) return -1;  // Error occurred
        data[i] = I2C1_MDR_R;  // Read the byte
    }

    return 0;  // Success
}

// Function to read the X-axis linear acceleration
int16_t read_linear_acceleration_x(uint8_t device_addr) {
    uint8_t data[2];  // Buffer for X-axis LSB and MSB
    if (I2C1_Read(device_addr, 0x28, data, 2) != 0) {
        return 0;  // Return 0 if there's an error
    }
    return ((int16_t)data[1] << 8) | data[0];  // Combine MSB and LSB
}

// Function to read the Y-axis linear acceleration
int16_t read_linear_acceleration_y(uint8_t device_addr) {
    uint8_t data[2];  // Buffer for Y-axis LSB and MSB
    if (I2C1_Read(device_addr, 0x2A, data, 2) != 0) {
        return 0;  // Return 0 if there's an error
    }
    return ((int16_t)data[1] << 8) | data[0];  // Combine MSB and LSB
}



///FILTERING//////////


// Update the moving average filter buffer
void update_moving_average(int16_t *buffer, int16_t new_value) {
    buffer[index] = new_value;
}

// Compute the moving average
int16_t get_moving_average(int16_t *buffer) {
    int32_t sum = 0;
         int i;
    for (i = 0; i < FILTER_SIZE; i++) {
        sum += buffer[i];
    }
    return sum / FILTER_SIZE;
}

// Apply low-pass filter
int16_t apply_low_pass_filter(int16_t current_value, int16_t *prev_filtered) {
    int16_t filtered_value = (int16_t)(ALPHA * current_value + (1 - ALPHA) * (*prev_filtered));
    *prev_filtered = filtered_value;
    return filtered_value;
}

// Combine moving average and low-pass filter
int16_t combine_filters(int16_t raw_value, int16_t *buffer, int16_t *prev_filtered) {
    // Update moving average
    update_moving_average(buffer, raw_value);
    int16_t moving_avg_value = get_moving_average(buffer);

    // Apply low-pass filter to moving average
    return apply_low_pass_filter(moving_avg_value, prev_filtered);
}


////////////SCALING///////////////

// Function to scale the raw acceleration values
float scale_acceleration(int16_t raw_value) {
    return raw_value * 0.001192;  // Scale factor for ±4g in m/s²
}

// Function to read and scale the X-axis linear acceleration
float read_linear_acceleration_x_scaled(uint8_t device_addr) {
    int16_t raw_value = read_linear_acceleration_x(device_addr);
    return scale_acceleration(raw_value);
}

// Function to read and scale the Y-axis linear acceleration
float read_linear_acceleration_y_scaled(uint8_t device_addr) {
    int16_t raw_value = read_linear_acceleration_y(device_addr);
    return scale_acceleration(raw_value);
}

*


-----------------------------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"

#define M_PI 3.14159265358979323846

// Robot state
typedef struct {
    float x;    // X-coordinate in mm
    float y;    // Y-coordinate in mm
    float yaw;  // Orientation in degrees
} RobotState;

// Function prototypes
void robot_init(oi_t *oi, RobotState *state);
void robot_update(oi_t *oi, RobotState *state);
void drift_correction(oi_t *oi, float target_yaw);
void update_localization(RobotState *state, float distance, float yaw_change);

void I2C1_Init(void);
int I2C1_Write(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
int I2C1_Read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

// Function to read yaw from the IMU
float read_yaw_from_imu(void);

int main(void) {
    oi_t *oi = oi_alloc();       // Allocate memory for the Roomba interface
    RobotState state = {0};      // Initialize robot state (x = 0, y = 0, yaw = 0)

    // Initialize hardware and sensors
    I2C1_Init();
    lcd_init();
    timer_init();
    robot_init(oi, &state);

    // Command the robot to move forward
    oi_setWheels(200, 200);  // Move forward at 200 mm/s

    while (1) {
        // Update sensor data and robot state
        robot_update(oi, &state);

        // Correct drift
        drift_correction(oi, 0);  // Target yaw is 0 for straight-line movement

        // Display robot position on LCD
        lcd_printf("Position:\nX: %.2f mm\nY: %.2f mm\nYaw: %.2f°",
                   state.x, state.y, state.yaw);

        // Add delay for real-time control loop
        timer_waitMillis(100);
    }

    // Stop the robot and clean up
    oi_setWheels(0, 0);
    oi_free(oi);

    return 0;
}

// Initialize I2C1 for communication
void I2C1_Init(void) {
    // Enable the clock for I2C1 and Port A
    SYSCTL_RCGCI2C_R |= 0x02;  // Enable I2C1 clock
    SYSCTL_RCGCGPIO_R |= 0x01;  // Enable GPIO Port A clock

    while ((SYSCTL_PRI2C_R & 0x02) == 0) {};  // Wait for I2C1 clock to stabilize
    while ((SYSCTL_PRGPIO_R & 0x01) == 0) {}; // Wait for Port A clock to stabilize

    // Configure PA6 (SCL) and PA7 (SDA) for I2C
    GPIO_PORTA_AFSEL_R |= 0xC0;  // Enable alternate functions on PA6, PA7
    GPIO_PORTA_ODR_R |= 0x80;    // Enable open drain on PA7 (SDA)
    GPIO_PORTA_DEN_R |= 0xC0;    // Enable digital functions on PA6, PA7
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0x00FFFFFF) | 0x33000000; // Assign I2C functions

    // Initialize I2C1 Master
    I2C1_MCR_R = 0x10;  // Set I2C Master mode
    I2C1_MTPR_R = 7;    // Set SCL clock speed to ~100kHz (assuming 16 MHz system clock)
}


// Write data to a specific register on the BNO055
int I2C1_Write(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
    I2C1_MSA_R = (device_addr << 1) & ~0x01;  // Set slave address and write mode

    I2C1_MDR_R = reg_addr;  // Write the register address
    I2C1_MCS_R = 0x03;      // Start and Run
    while (I2C1_MCS_R & 0x01) {};  // Wait for completion

    if (I2C1_MCS_R & 0x02) return -1;  // Error occurred

    // Write data bytes
         uint8_t i;
    for (i = 0; i < len; i++) {
        I2C1_MDR_R = data[i];  // Write the next byte
        I2C1_MCS_R = (i == (len - 1)) ? 0x05 : 0x01;  // Stop for the last byte
        while (I2C1_MCS_R & 0x01) {};  // Wait for completion
        if (I2C1_MCS_R & 0x02) return -1;  // Error occurred
    }

    return 0;  // Success
}

// Read data from a specific register on the BNO055
int I2C1_Read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
    // Send register address
    I2C1_MSA_R = (device_addr << 1) & ~0x01;  // Set slave address and write mode
    I2C1_MDR_R = reg_addr;  // Write the register address
    I2C1_MCS_R = 0x03;      // Start and Run
    while (I2C1_MCS_R & 0x01) {};  // Wait for completion
    if (I2C1_MCS_R & 0x02) return -1;  // Error occurred

    // Read data bytes
    I2C1_MSA_R = (device_addr << 1) | 0x01;  // Set slave address and read mode
        uint8_t i;
    for (i = 0; i < len; i++) {
        I2C1_MCS_R = (i == (len - 1)) ? 0x07 : 0x09;  // Stop for the last byte
        while (I2C1_MCS_R & 0x01) {};  // Wait for completion
        if (I2C1_MCS_R & 0x02) return -1;  // Error occurred
        data[i] = I2C1_MDR_R;  // Read the byte
    }

    return 0;  // Success
}


void robot_init(oi_t *oi, RobotState *state) {
    // Initialize Roomba
    oi_init(oi);

    // Initialize IMU in NDOF mode
    uint8_t data_write = 0x0C;  // NDOF mode
    I2C1_Write(0x29, 0x3D, &data_write, 1);

    // Calibrate IMU (wait until fully calibrated)
    uint8_t calib_status = 0;
    do {
        I2C1_Read(0x29, 0x35, &calib_status, 1);
    } while ((calib_status & 0x03) != 0x03);  // Wait for accelerometer calibration

    // Initialize robot state
    state->x = 0;
    state->y = 0;
    state->yaw = 0;  // Assume starting orientation is 0 degrees
}

void robot_update(oi_t *oi, RobotState *state) {
    // Update Roomba sensors
    oi_update(oi);

    // Read yaw from IMU
    float yaw = read_yaw_from_imu();

    // Calculate yaw change
    float yaw_change = yaw - state->yaw;

    // Update localization
    float distance = oi_getDistance(oi);  // Distance traveled in mm
    update_localization(state, distance, yaw_change);

    // Update current yaw in state
    state->yaw = yaw;
}

void drift_correction(oi_t *oi, float target_yaw) {
    // Read current yaw
    float current_yaw = read_yaw_from_imu();

    // Calculate error
    float error = target_yaw - current_yaw;

    // Simple correction
    if (fabs(error) > 2.0) {  // Allowable tolerance of 2 degrees
        if (error > 0) {
            oi_setWheels(200, 180);  // Slightly faster on left wheel
        } else {
            oi_setWheels(180, 200);  // Slightly faster on right wheel
        }
    } else {
        oi_setWheels(200, 200);  // Straight line
    }
}

void update_localization(RobotState *state, float distance, float yaw_change) {
    // Convert yaw_change to radians
    float yaw_rad = yaw_change * (M_PI / 180.0f);

    // Calculate displacement
    state->x += distance * cosf(yaw_rad);
    state->y += distance * sinf(yaw_rad);
}

float read_yaw_from_imu(void) {
    uint8_t data[2];
    I2C1_Read(0x29, 0x1A, data, 2);  // Yaw data registers
    int16_t raw_yaw = ((int16_t)data[1] << 8) | data[0];
    return raw_yaw / 16.0f;  // Convert to degrees (BNO055 scale factor)
}







#endif /* TEST_FILENAME_H_ */
