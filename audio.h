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

void Audio_Init(void);
void Audio_Play(void);
void Audio_Pause(void);
void Audio_Stop(void);
void Audio_Next(void);
void Audio_Specified_Song(uint8_t SNH, uint8_t SNL);


#endif /* AUDIO_H_ */
