/*
 * ping.c
 *
 *  Created on: Oct 28, 2024
 *      Author: mss3247
 */

#include "timer.h"
#include "ping.h"

volatile enum {LOW, HIGH, DONE} state; // set by ISR
volatile unsigned int rising_time; //Pulse start time: Set by ISR
volatile unsigned int falling_time; //Pulse end time: Set by ISR



void ping_init(void);
float ping_read(void);
void TIMER3B_Handler(void);
void send_pulse(void);

void configure_timer(){

    // Set up system clock
    SYSCTL_RCGCGPIO_R |= 0x00000002; //Enables the port’s system clock Pg.340
    timer_waitMillis(200);
    GPIO_PORTB_DIR_R |= 0x00000002; //Data direction register -> input
    GPIO_PORTB_DEN_R |= 0x00000008; //Digital enable register
    GPIO_PORTB_AFSEL_R |= 0x00000008; // Force Pin 3 to be Zero
    GPIO_PORTB_PCTL_R |= 0x00007000; // Set up Timer #3B T3CCP1 odd CCP pin Pg706



    // Configure Timer3B for input capture          // Note: Timer #3B T3CCP1 odd ccp pin //

                    SYSCTL_RCGCTIMER_R |= 0x00000008; // Pg 338 enable clock for Timer 3
               // Timer Programming Interface
                    timer_waitMillis(200);
                   // GPTMCTL: GPTM (General Purpose Timer) Control disable for set-up
                       TIMER3_CTL_R &= ~(0x00000100); // Timer B Event is disabled and will trigger on the positive edge

                   // GPTMCFG: GPTM Configuration
                       TIMER3_CFG_R = (TIMER3_CTL_R & ~(0x00000004)) | 0x00000004; // Set to 0x4 "split" timer B use 16-bit mode

                   // GPTMTnMR: GPTM Timer n Mode (n is A or B)
                       TIMER3_TBMR_R = (TIMER3_TBMR_R & ~(0x00000010)) | 0x00000007; // Set to Capture Mode "0x3"  pg. 729, 733 of data sheet

                   // GPTMTnILR: GPTM Timer n Interval Load
                       TIMER3_TBILR_R = 0xFFFF;
                       TIMER3_TBPR_R |= 0xFF;
                       TIMER3_CTL_R |= 0xC00; //pg. 737

                   // GPTMIMR: GPTM Interrupt Mask Register
                       TIMER3_IMR_R = (TIMER3_IMR_R & ~(0x00000400)) | 0x00000400; // Pg.745 Enable the Timer B Interrupt  p. 745 of data sheet


                   // GPTMICR: GPTM Interrupt Clear Register
                       TIMER3_ICR_R = (TIMER3_ICR_R & ~(0x00000400)) | 0x00000400; ; //Pg.754 Clears the Timer n Capture Mode Event Flag

                   // NVIC Enable
                       NVIC_EN1_R |= 0x0000010;

                   // GPTMCTL: GPTM (General Purpose Timer) Control enable for set-up
                       TIMER3_CTL_R |= 0x00000100; // Timer B Event is enabled and will trigger on the positive edge
                       IntRegister(INT_TIMER3B, TIMER3B_Handler);

                   //enable global interrupts
                       IntMasterEnable();

}

float ping_read(void) // Get distance from PING sensor
{
    float distance;
    float pulse_width, dist;

    send_pulse(); // Send short pulse to request PING burst


    // Wait for ISR to capture rising edge & falling edge time
    while (state != DONE){} // wait until the entire signal is recieved


    // Calculate the width of the pulse; convert to centimeters
    //Need to see if there is timer overflow
    if(falling_time > rising_time){
        //Need to account for the overflow
        pulse_width = rising_time + (0xFFFF - falling_time);
        pulse_width = pulse_width/16000000.0;
        //overflow_count++; IDK WHY COMMENTING THIS OUT MAKES IT WORK DONT ASK ME
    }

    else {
    pulse_width = (rising_time - falling_time)/16000000.0;
    }

    dist = ping_conversion(pulse_width);

    return dist;
}

float ping_conversion(float pulse_width){
    float distance;
    distance = (pulse_width * 34000)/2.0;
    return distance;
}



void send_pulse(void){
    // GPTMIMR: GPTM Interrupt Mask Register
        TIMER3_IMR_R = (TIMER3_IMR_R & ~(0x00000400)); // Pg.745 Enable the Mask -> Disable interrupts

     TIMER3_CTL_R &= ~(0x00000100); //pg. 737 Disable Timer B
     GPIO_PORTB_AFSEL_R &= ~(0x00000008); // Force Pin 3 to be Zero

    //SYSCTL_RCGCGPIO_R |= 0x00000002; //Enables the port’s system clock Pg.340 fixme
    GPIO_PORTB_DIR_R |= 0x00000008; //Data direction register -> input
   // GPIO_PORTB_DEN_R |= 0x00000008; //Digital enable register
    GPIO_PORTB_DATA_R &= ~(0x00000008);
    GPIO_PORTB_DATA_R |= 0x08;
     timer_waitMicros(5); // Send Trigger
     GPIO_PORTB_DATA_R &= ~(0x00000008);
     GPIO_PORTB_DIR_R  &= ~(0x00000008); //Data direction register -> output
     //GPIO_PORTB_DEN_R  &= ~(0x00000008); //Digital disable register

     //Reenable AFSEL

     GPIO_PORTB_AFSEL_R |=  (0x00000008); // Force Pin 3 to be 1
    // GPTMICR: GPTM Interrupt Clear Register
     TIMER3_ICR_R |= 0x00000400; ; //Pg.754 Clears the Timer n Capture Mode Event Flag

    // GPTMIMR: GPTM Interrupt Mask Register
     TIMER3_IMR_R = (TIMER3_IMR_R & ~(0x00000400)) | 0x00000400; // Pg.745 Disable the Mask -> Enable interrupts
     TIMER3_CTL_R |= (0x00000100); //pg. 737 Disable Timer B
     state = LOW; // set state to low


}

// ISR: Capture rising edge and falling edge time of PING sensor
void TIMER3B_Handler(void)
{

    int msb, lsb;
    msb = TIMER3_TBPS_R & 0xFF;
    lsb = TIMER3_TBR_R & 0xFFFF;

    //Check if a receive byte IRQ has occurred
    if(TIMER3_MIS_R & 0x00000400){

        if(state == LOW){
            //then it must be a rising edge
            rising_time = (msb << 16)  | lsb;
            state = HIGH;
        }
        else if(state == HIGH){
            //it must be a falling edge
            falling_time = (msb << 16)  | lsb;
            state = DONE;
        }


    TIMER3_ICR_R = 0x00000400; //clear the receive byte interrupt
    }





}
