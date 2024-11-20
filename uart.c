/*
*
*   uart_extra_help.c
* Description: This is file is meant for those that would like a little
*              extra help with formatting their code, and following the Data sheet.
*/

#include "uart.h"
#include "timer.h"
#define REPLACE_ME 0x00


void uart_init() //THIS SAID INT BAUD BEFORE I CHANGED IT
{
    SYSCTL_RCGCGPIO_R |= 0b00000010;      // enable clock GPIOB (page 340)
    SYSCTL_RCGCUART_R |= 0b00000010;      // enable clock UART1 (page 344)
    timer_waitMillis(50);
    GPIO_PORTB_AFSEL_R |= 0b00000011;      // sets PB0 and PB1 as peripherals (page 671)
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & ~0xFF) | 0x00000011;      // pmc0 and pmc1       (page 688)  also refer to page 650
    GPIO_PORTB_DEN_R   |= 0b00000011;      // enables pb0 and pb1
    GPIO_PORTB_DIR_R  = (GPIO_PORTB_DIR_R & ~0x02) | 0b00000001;      // sets pb0 as output, pb1 as input

    //compute baud values [UART clock= 16 MHz] 
    double fbrd;  //fractional baud rate
    int    ibrd;  // integer baud rate

    ibrd = 8; // integer(0.8507 * 64 + 0.5)
    fbrd = 44;

    UART1_CTL_R &= 0b1111111111111110;      // disable UART1 (page 918)
    UART1_IBRD_R = 0b1000;        // write integer portion of BRD to IBRD (8)
    UART1_FBRD_R = 0b00101100;   // write fractional portion of BRD to FBRD (44)

    UART1_LCRH_R &=   0xFFFFFF00;            //0b01100000;        // write serial communication parameters (page 916) * 8bit and no parity, one stop bit
    UART1_LCRH_R |=  0x00000060;
    UART1_CC_R   = 0x0;          // use system clock as clock source (page 939)
    UART1_CTL_R |= ~0b1111111111111110;        // enable UART1
}

void uart_sendChar(char data)
{
    // UART1_CTL_R |= 0b0000 0001 0000 0001; // enable Tx & disable Rx NOT NEEDED
   // Communicate from Putty terminal back to LCD, just one Char
     while(UART1_FR_R & 0b00100000){

     }
     // Send data
     UART1_DR_R = data;

}
char uart_receive(void)
{
    //UART1_CTL_R |= 0b0000 0010 0000 0001; // enable Rx & disable Tx NOT NEEDED
    char data = 0;
 
    while(UART1_FR_R & 0b00010000){ // keep waiting as long as FIFO is empty
    }


    // mask the 4 error bits and grab only 8 data bits
    data = (char)(UART1_DR_R & 0x0FF); //Data Bits & No parity, One stop, 8 bits transfered
    return data;
}


void uart_sendStr(const char *data)
{

    int i;
    for(i = 0; i < 20; i++){
        uart_sendChar(data[i]);
    }
}
// _PART3


void uart_interrupt_init()
{

    UART1_CTL_R &= 0xFFFE; //UART Control to disable UART

    UART1_ICR_R |= 0x0010; // Clear all Interrupts flags

    // Enable interrupts for receiving bytes through UART1
    //enable interrupt on receive - page 924 (ENABLE PIN 4)

    UART1_IM_R |= 0x0010; // ENABLE UART1 to receive & send interrupts

    //Set a priority of usart1 interrupt
  // NVIC_PRI1_R &= 0xFF0FFFFF; //FIX ME
  // NVIC_PRI1_R |= 0x00200000;

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.

    //enable uart1 interrupts - page 104 
    // UART1 is interrupt number 22, which corresponding bit register is 6
    NVIC_EN0_R |= 0x00000040; // Enable interrupts in the NVIC

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    //give the micro-controller the address of our interrupt handler - page 104 22 is the vector number
    // using the starting location of uart_int
    IntRegister(INT_UART1, uart_interrupt_handler); // Register the UART1 interrupt handler

    // Enable global interrupts (found in slides)
    IntMasterEnable();

    UART1_CTL_R |= 0x0001; //UART Control to enable UART
}

void uart_interrupt_handler()
{
    // STEP1: Check the Masked Interrupt Status
    if(UART1_MIS_R & 0x0010){ //check if UART recieved a masked interrupt status, if 1 an unmasked interrupt was signaled due to passing through a specified received FIFO level

        //STEP2:  Copy the data (store in the global variable)
        uart_data = UART1_DR_R & 0xFF; // copy data from UART1 data register

        //STEP3:  Clear the interrupt
         UART1_ICR_R |= 0x0010; //Clearing the RXMIS register in case another event occurs

         //Also set a flag for main
         flag1 = 1;
    }

}
