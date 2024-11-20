/*
 * adc.h
 *
 *  Created on: Oct 24, 2024
 *      Author: mss3247
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inc/tm4c123gh6pm.h>
#include "timer.h"


void adc_init(void);
int adc_read(void);
float clean_IR_val(void);


#endif /* ADC_H_ */
