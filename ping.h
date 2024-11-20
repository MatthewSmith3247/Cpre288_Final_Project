/*
 * ping.h
 *
 *  Created on: Oct 28, 2024
 *      Author: mss3247
 */

#ifndef PING_H_
#define PING_H_
extern volatile char flag;
extern volatile unsigned int overflow_count;

void ping_init(void);
float ping_read(void);
void configure_timer(void);
void send_pulse(void);
void TIMER3B_Handler(void);
float ping_conversion(float pulse_width);

#endif /* PING_H_ */
