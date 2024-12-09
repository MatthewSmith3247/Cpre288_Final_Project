/*
 * audio.c
 *
 *  Created on: Nov 13, 2024
 *      Author: Matthew Smith
 *      Hardware: DY-SV5W Audio Module
 *      Communication: UART (Dip Switch in Mode 3)
 */

#include "timer.h"
#include "audio.h"

// Function to initialize UART for DY-SV5W audio module and play an MP3 file once

void Audio_Init(void) {
    // Step 1: Enable UART3 and GPIO Port C clocks PC6 is Rx and PC7 is Tx
    SYSCTL_RCGCUART_R |= 0b001000;    // Enable UART3 clock (p. 344)
    SYSCTL_RCGCGPIO_R |= 0b000100;    // Enable GPIO Port C clock (p. 340)
    timer_waitMillis(50);

    // Step 2: Configure GPIO pins for UART
    GPIO_PORTC_AFSEL_R |= 0xC0;        // Enable alternate function on PC6, PC7 (p. 1351)
    GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R & 0x00FFFFFF) | 0x11000000; // Configure PC0, PC1 for UART (p. 1351)
    GPIO_PORTC_DEN_R |= 0xC0;          // Enable digital function on PC6, PC7 (p. 682)
    GPIO_PORTC_DIR_R = (GPIO_PORTC_DIR_R & ~(0x80)) |0x80;   //PC6 is input(RX) PC7 is output(TX)

    // Step 3: Set Baud Rate for UART3
    UART3_CTL_R &= ~0x0001;            // Disable UART3 during setup (p. 918)
    UART3_IBRD_R = 104;                // Integer part of baud rate for 9600 bps with 16 MHz system clock (p. 904)
    UART3_FBRD_R = 11;                 // Fractional part of baud rate (p. 904)

    // Step 4: Configure UART frame format
    UART3_LCRH_R = (UART3_LCRH_R & 0xFFFFFF00) | 0x00000060;     // 8-bit, enable FIFO (p. 905) 8-bit, no parity, one stop bit
    UART3_CC_R = 0x0; //use system clock as clock source (page 939)
    UART3_CTL_R |= 0x0101;            // Enable UART2 (p. 918)



    //Audio_Play();  // Example command to play the first audio file; replace with actual play command as needed
}

// function to send command to DY-SV5W module?
void Audio_Play(void) {
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    timer_waitMillis(2);
    UART3_DR_R = 0xAA;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    timer_waitMillis(2);

    UART3_DR_R = 0x02;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    timer_waitMillis(2);
    UART3_DR_R = 0x00;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    timer_waitMillis(2);

    UART3_DR_R = 0xAC;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    timer_waitMillis(2);
}


void Audio_Pause(void) {
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0xAA;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0x03;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0x00;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0xAD;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
}


void Audio_Stop(void) {
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0xAA;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0x04;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0x00;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0xAE;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
}


void Audio_Next(void) {
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0xAA;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0x06;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0x00;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0xB0;
    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
}


void Audio_Specified_Song(uint8_t SNH, uint8_t SNL) {

    uint8_t check_sum = (0xAA + 0x07 + 0x02 + SNH + SNL) & 0xFF;  // Mask to retain low 8 bits


    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0xAA;

    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = 0x07;

    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }

    UART3_DR_R = 0x02;

    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }

    UART3_DR_R = SNH; //I DONT KNOW WHY BUT ALWAYS HAVE THIS VALUE OF 0x00 and just enter in the SONG MACRO in the 2nd Paramter. It Just works.

    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = SNL;

    while ((UART3_FR_R & 0x20) != 0);{
        // Wait if TX buffer is full (p. 904)
    }
    UART3_DR_R = check_sum;

        while ((UART3_FR_R & 0x20) != 0);{
            // Wait if TX buffer is full (p. 904)


    //Audio_Play();

    }
}

