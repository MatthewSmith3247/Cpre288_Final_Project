/*
 * IMU.h
 *
 *  Created on: Dec 5, 2024
 *      Author: mss3247
 */

#ifndef IMU_H_
#define IMU_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
#include "math.h"


static void addr_set(uint8_t addr);

/** BNO055 Address A **/
#define BNO055_ADDRESS_A (0x28)

/** BNO055 Address B **/
#define BNO055_ADDRESS_B (0x29)

/** BNO055 ID **/
#define BNO055_ID (0xA0)


void I2C1_Init(void);
void BNO055_Init(void);

int I2C1_Write(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
int I2C1_Read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

//Linear Acceleration Readings
int16_t read_linear_acceleration_x(uint8_t device_addr);
int16_t read_linear_acceleration_y(uint8_t device_addr);
int16_t read_linear_acceleration_z(uint8_t device_addr);

// Magnetometer Data
int16_t read_mag_x(uint8_t device_addr);
int16_t read_mag_y(uint8_t device_addr);
int16_t read_mag_z(uint8_t device_addr);

// Gravity Vector Data
int16_t read_grav_vec_x(uint8_t device_addr);
int16_t read_grav_vec_y(uint8_t device_addr);
int16_t read_grav_vec_z(uint8_t device_addr);

// Heading, Roll, pitch
int16_t read_euler_heading(uint8_t device_addr);
int16_t read_euler_roll(uint8_t device_addr);
int16_t read_euler_pitch(uint8_t device_addr);



#endif /* IMU_H_ */
