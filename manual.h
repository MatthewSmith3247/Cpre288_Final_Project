/*
 * manual.h
 *
 *  Created on: Dec 7, 2024
 *      Author: jreiff
 */

#ifndef MANUAL_H_
#define MANUAL_H_
//INCLUDE ALL THE INCLUDES
#include "adc.h" //YES
#include "uart.h" //YES
#include "Timer.h" //YES
#include "lcd.h" //YES
#include "math.h" //JUST MATH SO YES
#include "open_interface.h" //YES
#include "movement.h" //YES
#include "button.h" //YES
#include "ping.h" //YES
#include "servo.h" //NO? THEY DO IT DIFFERENTLY
#include "adc.h" //YES
#include "methods.h" //YES
#include "audio.h" //YES
#include "IMU.h"

void manualDriver(oi_t *sensor_data);
void move_forward_detect(oi_t *sensor_data, int distance);
int fastScanManual(int startDeg, int endDeg);
int fullScanManual(int startDeg, int endDeg);
int fullScanManualNoPlot(int startDeg, int endDeg);



#endif /* MANUAL_H_ */
