/*
 * IMU.c
 *
 *  Created on: Dec 5, 2024
 *      Author: Matthew Smith
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
#include "math.h"
#include "IMU.h"



/**
 * Sets the slave address of IMU
 *
 * @param addr either 0x28 or 0x29
 */
void addr_set(uint8_t addr)
{
switch (addr)
{
case (0x28):
    GPIO_PORTB_DATA_R &= ~0x80;
    break;
case (0x29): // FALL THROUGH
default:
    GPIO_PORTB_DATA_R |= 0x80;
}
}

// Initialize I2C1 for communication
void I2C1_Init(void)
{
// Enable the clock for I2C1 and Port A
SYSCTL_RCGCI2C_R |= 0x02;  // Enable I2C1 clock
SYSCTL_RCGCGPIO_R |= 0x03;  // Enable GPIO Port A and B clock

while ((SYSCTL_PRI2C_R & 0x02) == 0)
{
};  // Wait for I2C1 clock to stabilize
while ((SYSCTL_PRGPIO_R & 0x03) == 0)
{
}; // Wait for Port A clock to stabilize

// Configure PA6 (SCL) and PA7 (SDA) for I2C
GPIO_PORTA_DIR_R |= 0xC0;
GPIO_PORTA_AFSEL_R |= 0xC0;  // Enable alternate functions on PA6, PA7
GPIO_PORTA_ODR_R |= 0x80;    // Enable open drain on PA7 (SDA)
GPIO_PORTA_DEN_R |= 0xC0;    // Enable digital functions on PA6, PA7
GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0x00FFFFFF) | 0x33000000; // Assign I2C functions

//Configure the Address and Reset Lines on GPIO PortB
GPIO_PORTB_DEN_R |= 0xC4;
GPIO_PORTB_AFSEL_R &= ~0xC0; //FIXME maybe
GPIO_PORTB_DIR_R = (GPIO_PORTB_DIR_R & ~0x4) | 0xC0;

// Configure the ADD and RESET Lines
// Initialize I2C1 Master
I2C1_MCR_R = 0x10;  // Set I2C Master mode
I2C1_MTPR_R = 0x7; // Set SCL clock speed to ~100kHz (assuming 16 MHz system clock)

// Set PB6 to HIGH
GPIO_PORTB_DATA_R |= 0x40;

addr_set(BNO055_ADDRESS_B);

}

void BNO055_Init(void)
{
// Reset the device

    uint8_t reset_cmd = 0x20;
    I2C1_Write(BNO055_ADDRESS_B, 0x3F, &reset_cmd, 1);
    timer_waitMillis(650);  // Wait for reset to complete

// Set to CONFIG mode first
    uint8_t config_mode = 0x00;
    I2C1_Write(BNO055_ADDRESS_B, 0x3D, &config_mode, 1);
    timer_waitMillis(25);

// Configure page
    uint8_t page_zero = 0x00;
    I2C1_Write(BNO055_ADDRESS_B, 0x07, &page_zero, 1);
    timer_waitMillis(10);

    uint8_t unitsel = 0x00; // Default settings (Euler angles in degrees)
    I2C1_Write(BNO055_ADDRESS_B, 0x3B, &unitsel, 1);
    timer_waitMillis(10);

    uint8_t axis_config = 0x21; // Setting to P0
    uint8_t axis_sign_config = 0x04; // Setting to P0

    I2C1_Write(BNO055_ADDRESS_B, 0x41, &axis_config, 1);
    timer_waitMillis(10);

    I2C1_Write(BNO055_ADDRESS_B, 0x42, &axis_sign_config, 1);
    timer_waitMillis(10);

// Then set to NDOF mode
    uint8_t ndof_mode = 0x0C;
    I2C1_Write(BNO055_ADDRESS_B, 0x3D, &ndof_mode, 1);
    timer_waitMillis(650);

// Wait for sensor to be fully calibrated
    uint8_t cal_status = 0;
    uint8_t sys_cal = 0, gyro_cal = 0, accel_cal = 0, mag_cal = 0;

    lcd_printf("Calibrating...\n");
    while (sys_cal != 3 || gyro_cal != 3 || mag_cal != 3)
    {
        // Read the calibration status
        I2C1_Read(BNO055_ADDRESS_B, 0x35, &cal_status, 1);
        sys_cal = (cal_status >> 6) & 0x03;
        gyro_cal = (cal_status >> 4) & 0x03;
        accel_cal = (cal_status >> 2) & 0x03;
        mag_cal = cal_status & 0x03;

        // Display calibration status on LCD
        lcd_printf("Calibrating...\nSys:%d G:%d A:%d M:%d", sys_cal, gyro_cal,
                   accel_cal, mag_cal);

        // Wait for a short period before checking again
        timer_waitMillis(500);
    }
// Calibration is complete
    lcd_printf("Calibration Complete\nSys:%d G:%d A:%d M:%d", sys_cal, gyro_cal,
               accel_cal, mag_cal);

    timer_waitMillis(1000);

// Check system status
    uint8_t sys_status;
    I2C1_Read(BNO055_ADDRESS_B, 0x39, &sys_status, 1);
    timer_waitMillis(50);

    if(sys_status == 0x1){
        while(1){
        lcd_printf("System Error");
        }
    }
// Check system error
    uint8_t sys_err;
    I2C1_Read(BNO055_ADDRESS_B, 0x3A, &sys_err, 1);
    timer_waitMillis(50);

}

// Write data to a specific register on the BNO055
int I2C1_Write(uint8_t device_addr, uint8_t reg_addr, uint8_t *data,
               uint8_t len)
{

if (len == 0)
{ // Data did not send
    return 0;
}

I2C1_MSA_R = (device_addr << 1) | (I2C1_MSA_R & ~0x01); //Set slave address and write mode

I2C1_MDR_R = (I2C1_MDR_R & 0x00) | (reg_addr & 0xFF); // Write the register address and clear the previous version

I2C1_MCS_R = 0x03;      // Start and Run
while (I2C1_MCS_R & 0x01)
{
};  // Wait for Data Ack
while (I2C1_MCS_R & 0x0C)
{
};  // Wait for Address Ack

// Write data bytes
uint8_t i;
for (i = 0; i < len; ++i)
{
    I2C1_MDR_R = (I2C1_MDR_R & 0x00) | (data[i] & 0xFF);
    ;

    I2C1_MCS_R = 0x01;      // Start and Run
    while (I2C1_MCS_R & 0x01)
    {
    };  // Wait for Data Ack
    while (I2C1_MCS_R & 0x0C)
    {
    };  // Wait for Address Ack
}

I2C1_MCS_R = 0x04;      // Start and Run
while (I2C1_MCS_R & 0x01)
{
};  // Wait for Data Ack
while (I2C1_MCS_R & 0x0C)
{
};  // Wait for Address Ack

return 0;  // Success
}

// Read data from a specific register on the BNO055
int I2C1_Read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{

if (len == 0)
{ // Data did not send
    return 0;
}

I2C1_MSA_R = (device_addr << 1) | (I2C1_MSA_R & ~0x01); //Set slave address and write mode

I2C1_MDR_R = (I2C1_MDR_R & 0x00) | (reg_addr & 0xFF); // Write the register address

I2C1_MCS_R = 0x03;      // Start and Run

while (I2C1_MCS_R & 0x01)
{
};  // Wait for Data Ack
while (I2C1_MCS_R & 0x0C)
{
};  // Wait for Address Ack

// Read data bytes
I2C1_MSA_R = (device_addr << 1) | 0x01;  // Set slave address and read mode

uint8_t i = 0;

if (len == 1)
{  // determine if we are sending 1 byte or more
    I2C1_MCS_R = 0x03;      // Start and Run (1 Byte)
    while (I2C1_MCS_R & 0x01)
    {
    };  // Wait for Data Ack
    I2C1_MCS_R = 0x04;      // Stop

}
else
{
    I2C1_MCS_R = 0x0B;      // Start and Ack

    while (i <= len)
    {

        while (I2C1_MCS_R & 0x01)
        {
        };   // Wait for Data Ack
        I2C1_MCS_R = 0x09; // Receive Operation Mater remains in master receive state.
    }

    while (I2C1_MCS_R & 0x01)
    {
    };
    I2C1_MCS_R = 0x05;
}

while (I2C1_MCS_R & 0x01)
{
};
data[i] = (I2C1_MDR_R & 0xFF);

i++;

return 0;  // Success
}

// Function to read the X-axis linear acceleration
int16_t read_linear_acceleration_x(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x28, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x29, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];

}
avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;
}

// Function to read the Y-axis linear acceleration
int16_t read_linear_acceleration_y(uint8_t device_addr)
{
uint8_t data[2];  // Buffer for Y-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x2A, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x2B, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

int16_t read_linear_acceleration_z(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x2C, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x2D, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

int16_t read_mag_x(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0xE, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0xF, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

int16_t read_mag_y(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x10, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x11, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

int16_t read_mag_z(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x12, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x13, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

int16_t read_grav_vec_x(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x2E, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x2F, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

int16_t read_grav_vec_y(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x30, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x31, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

int16_t read_grav_vec_z(uint8_t device_addr)
{

uint8_t data[2];  // Buffer for X-axis LSB and MSB
int16_t avg[5] = { 0 };
int16_t avg_sum = 0;
int i;

for (i = 0; i < 5; i++)
{
    I2C1_Read(device_addr, 0x32, data, 1);
    timer_waitMillis(5);
    I2C1_Read(device_addr, 0x33, data + 1, 1);

    avg[i] = ((int16_t) data[1] << 8) | data[0];
}

avg_sum = (int16_t) (avg[0] + avg[1] + avg[2] + avg[3] + avg[4]) / 5;

return avg_sum;  // Combine MSB and LSB
}

// Implementation
int16_t read_euler_heading(uint8_t device_addr)
{
    uint8_t data[2];
    I2C1_Read(device_addr, 0x1A, data, 1); // LSB
    timer_waitMillis(1);
    I2C1_Read(device_addr, 0x1B, data + 1, 1); // MSB
    return ((int) data[1] << 8) | data[0];
}

int16_t read_euler_roll(uint8_t device_addr)
{
uint8_t data[2];
I2C1_Read(device_addr, 0x1C, data, 1); // LSB
timer_waitMillis(1);
I2C1_Read(device_addr, 0x1D, data + 1, 1); // MSB
return ((int16_t) data[1] << 8) | data[0];
}

int16_t read_euler_pitch(uint8_t device_addr)
{
uint8_t data[2];
I2C1_Read(device_addr, 0x1E, data, 1); // LSB
timer_waitMillis(1);
I2C1_Read(device_addr, 0x1F, data + 1, 1); // MSB
return ((int16_t) data[1] << 8) | data[0];
}

