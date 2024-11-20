/*
 * audio.c
 *
 *  Created on: Nov 13, 2024
 *      Author: mss3247
 */

#include "timer.h"
#include "audio.h"

// Function to initialize UART for DY-SV5W audio module and play an MP3 file once
void Audio_Init(void) {
    // Step 1: Enable UART2 and GPIO Port D clocks PD6 is Rx and PD7 is Tx
    SYSCTL_RCGCUART_R |= 0x04;  // Enable UART2 clock (p. 344)
    SYSCTL_RCGCGPIO_R |= 0x08;    // Enable GPIO Port D clock (p. 340)
    timer_waitMillis(50);

    // Step 2: Configure GPIO pins for UART
    GPIO_PORTD_AFSEL_R |= 0xC0;        // Enable alternate function on PD6, PD7 (p. 1351)
    GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R & 0x00FFFFFF) | 0x11000000; // Configure PB0, PB1 for UART (p. 1351)
    GPIO_PORTD_DEN_R |= 0xC0;          // Enable digital function on PD6, PD7 (p. 682)
    GPIO_PORTD_DIR_R = (GPIO_PORTD_DIR_R & ~(0x80)) |0x80;   //PD6 is input PD7 is output

    // Step 3: Set Baud Rate for UART1
    UART2_CTL_R &= ~0x0001;            // Disable UART6 during setup (p. 918)
    UART2_IBRD_R = 104;                // Integer part of baud rate for 9600 bps with 16 MHz system clock (p. 904)
    UART2_FBRD_R = 11;                 // Fractional part of baud rate (p. 904)

    // Step 4: Configure UART frame format
    UART2_LCRH_R = (UART2_LCRH_R & 0xFFFFFF00) | 0x00000060;         //(UART_LCRH_WLEN_8 | UART_LCRH_FEN);  // 8-bit, enable FIFO (p. 905) 8-bit, no parity, one stop bit
    UART2_CC_R = 0x0; //use system clock as clock source (page 939)
    UART2_CTL_R |= 0x0001;            // Enable UART2 (p. 918)


    // Step 5: Enable UART module
   // UART2_CTL_R |= (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);  // Enable UART, TX, RX (p. 905)

    // Step 6: Send Play Command to DY-SV5W for "test" MP3 file

    // Assuming "test" MP3 is at address 0x01 or identified by a specific command <- trial an error with this
    Audio_SendCommand(0x0F);  // Example command to play the first audio file; replace with actual play command as needed
}

// function to send command to DY-SV5W module?
void Audio_SendCommand(uint8_t command) {
    while ((UART2_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART2_DR_R = command;  // Send command byte (p. 904)
}


