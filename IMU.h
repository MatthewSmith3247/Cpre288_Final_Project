/*
 * IMU.h
 *
 *  Created on: Dec 5, 2024
 *      Author: Matthew Smith
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


// Start Page 0 Register defines
#define IMU_CHIP_ID         0x00
#define IMU_ACC_ID          0x01
#define IMU_MAG_ID          0x02
#define IMU_GYR_ID_R        0x03
#define IMU_SW_REV_MSB      0x04
#define IMU_SW_REV_LSB      0x05
#define IMU_BL_VER_R        0x06
#define IMU_PAGE_ID         0x07
#define IMU_ACC_DATAX_LSB   0x08
#define IMU_ACC_DATAX_MSB   0x09
#define IMU_ACC_DATAY_LSB   0x0A
#define IMU_ACC_DATAY_MSB   0x0B
#define IMU_ACC_DATAZ_LSB   0x0C
#define IMU_ACC_DATAZ_MSB   0x0D
#define IMU_MAG_DATAX_LSB   0x0E
#define IMU_MAG_DATAX_MSB   0x0F
#define IMU_MAG_DATAY_LSB   0x10
#define IMU_MAG_DATAY_MSB   0x11
#define IMU_MAG_DATAZ_LSB   0x12
#define IMU_MAG_DATAZ_MSB   0x13
#define IMU_GYR_DATAX_LSB   0x14
#define IMU_GYR_DATAX_MSB   0x15
#define IMU_GYR_DATAY_LSB   0x16
#define IMU_GYR_DATAY_MSB   0x17
#define IMU_GYR_DATAZ_LSB   0x18
#define IMU_GYR_DATAZ_MSB   0x19
#define IMU_EUL_HEAD_LSB    0x1A
#define IMU_EUL_HEAD_MSB    0x1B
#define IMU_EUL_ROLL_LSB    0x1C
#define IMU_EUL_ROLL_MSB    0x1D
#define IMU_EUL_PTCH_LSB    0x1E
#define IMU_EUL_PTCH_MSB    0x1F
#define IMU_QUA_DATAW_LSB   0x20
#define IMU_QUA_DATAW_MSB   0x21
#define IMU_QUA_DATAX_LSB   0x22
#define IMU_QUA_DATAX_MSB   0x23
#define IMU_QUA_DATAY_LSB   0x24
#define IMU_QUA_DATAY_MSB   0x25
#define IMU_QUA_DATAZ_LSB   0x26
#define IMU_QUA_DATAZ_MSB   0x27
#define IMU_LIA_DATAX_LSB   0x28
#define IMU_LIA_DATAX_MSB   0x29
#define IMU_LIA_DATAY_LSB   0x2A
#define IMU_LIA_DATAY_MSB   0x2B
#define IMU_LIA_DATAZ_LSB   0x2C
#define IMU_LIA_DATAZ_MSB   0x2D
#define IMU_GRV_DATAX_LSB   0x2E
#define IMU_GRV_DATAX_MSB   0x2F
#define IMU_GRV_DATAY_LSB   0x30
#define IMU_GRV_DATAY_MSB   0x31
#define IMU_GRV_DATAZ_LSB   0x32
#define IMU_GRV_DATAZ_MSB   0x33
#define IMU_TEMP            0x34
#define IMU_CALIB_STAT      0x35
#define IMU_SYS_STATUS      0x39
#define IMU_UNIT_SEL        0x3B
#define IMU_OPR_MODE        0x3D


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
