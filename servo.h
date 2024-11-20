/*
 * servo.h
 *
 *  Created on: Nov 3, 2024
 *      Author: mss3247
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inc/tm4c123gh6pm.h>
#include "timer.h"
#include "lcd.h"

#ifndef SERVO_H_
#define SERVO_H_

 void servo_init (void);
 void servo_move(int servo_degree);
 void servo_calibrate(void);



#endif /* SERVO_H_ */
