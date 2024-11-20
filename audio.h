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
void Audio_SendCommand(uint8_t command);


#endif /* AUDIO_H_ */
