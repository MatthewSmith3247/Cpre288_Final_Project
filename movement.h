/*
 * movement.h
 *
 *  Created on: Sep 11, 2024
 *      Author: stanley9
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_
#include "open_interface.h"

#define DEGREE_ACCURACY 0.5
#define MM_ACCURACY 1

void moveForward(oi_t *sensor, float distance_cm);
void moveBackward(oi_t *sensor, float distance_cm);
void turnClockwise(oi_t *sensor, float degrees);
void turnCounterClockwise(oi_t *sensor, float degrees);

#endif /* MOVEMENT_H_ */
