/*
 * audio.h
 *
 *  Created on: Nov 13, 2024
 *      Author: mss3247
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"



//Defining the Macros for Audio Specified Song _LB _HB
                                                       // Time to wait for after playing the mp3 in secounds
#define Good_Morning                             0x01  //time = 6
#define Are_You_Ready                            0x08  //time = 2
#define Im_Moving_Fwd                            0x09  //time = 2
#define Moving_Fwd                               0x0A  //time = 2
#define Im_Stopping_Now                          0x0B  //time = 2
#define Stopping_Now                             0x0C  //time = 2
#define There_are_some_obstacles_ahead           0x0D  //time = 3
#define Let_Turn_Left                            0x0E  //time = 2
#define Im_Turning_to_the_right                  0x0F  //time = 3
#define Lets_Turn_Right                          0x10  //time = 3
#define Im_Turning_to_the_left                   0x02  //time = 3
#define wall                                     0x03  //time = 6
#define Black_hole                               0x04  //time = 7
#define We_have_reached_the_Bathroom             0x05  //time = 4
#define We_have_reached_the_Kitchen              0x06  //time = 4
#define We_have_reached_the_Living_Room          0x07  //time = 6
#define We_have_reached_the_front_door           0x11  //time = 5
#define end                                      0x12  //time = 39
#define Joke_1                                   0x13  //time = 6
#define Joke_2                                   0x14  //time = 6
#define Joke_3                                   0x15  //time = 7
#define Small_Obj_Hit                            0x18  //time = 5
#define Med_Obj_Hit                              0x16  //time = 5
#define Large_Obj_Hit                            0x17  //time = 5



void Audio_Init(void);
void Audio_Play(void);
void Audio_Pause(void);
void Audio_Stop(void);
void Audio_Next(void);
void Audio_Specified_Song(uint8_t SNH, uint8_t SNL); //<----NOTE: JUST ENTER IN 0x00 for the SNH value and enter in any of the macros for the 2nd parameter.


#endif /* AUDIO_H_ */
