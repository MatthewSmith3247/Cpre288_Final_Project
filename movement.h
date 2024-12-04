/*
 * movement.h
 *
 *  Created on: Sep 11, 2024
 *      Author: stanley9
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_


void moveForward(oi_t*sensor, int centimeters);
void moveBackward(oi_t*sensor, int centimeters);
void turnClockwise(oi_t *sensor, float degrees);
void turnCounterClockwise(oi_t *sensor, float degrees);


#endif /* MOVEMENT_H_ */
