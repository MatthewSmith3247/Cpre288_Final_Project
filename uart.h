/*
*   uart.h
*   Used to set up the UART [UART1 at 115200]
*
 * Description: This is file is meant for those that would like a little
 *              extra help with formatting their code.
*/

#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

// These two variables have been declared
// in the file containing main
 volatile  char uart_data;  // Your UART interupt code can place read data here
 volatile  char flag1;       // Your UART interupt can update this flag
                                  // to indicate that it has placed new data
                                  // in uart_data       


void uart_init(int baud);

void uart_sendChar(char data);

char uart_receive(void);

void uart_sendStr(const char *data);

void uart_interrupt_init();

void uart_interrupt_handler();

void connectToGui(void);

void cybot_send_string(char string[50]);

#endif /* UART_H_ */
