/*
 * adc.c
 *
 *  Created on: Oct 24, 2024
 *      Author: mss3247
 */


#include "adc.h"
#include "math.h"


    //ADC Configuration
    void adc_init(void){

        // GPIO Registers
        SYSCTL_RCGCGPIO_R |= 0b011010;// Set Clocks for Ports B,D,E (Run Mode Clock Gating Control) Pg.340
        timer_waitMillis(10);
        GPIO_PORTB_AFSEL_R = (GPIO_PORTB_AFSEL_R & (~0x00000010)) | 0x00000010; // Take in Port B Pin 4 of the ASFEL
        timer_waitMillis(10);
        GPIO_PORTB_DIR_R = (GPIO_PORTB_DIR_R & (~0x00000010));     // 4) make PE4 input Pg.663
        timer_waitMillis(10);
        GPIO_PORTB_DEN_R = (GPIO_PORTB_DEN_R & (~0x00000010));     // 6) disable digital I/O on PB4 Pg.683
        timer_waitMillis(10);
        GPIO_PORTB_AMSEL_R = (GPIO_PORTB_AMSEL_R & (~0x00000010))| 0x00000010;   // 7) enable analog functionality on PB4 (A10) Pg.687
        timer_waitMillis(10);
       //ADC 0 Registers
        SYSCTL_RCGCADC_R |= 0b01;// Set Clocks for Ports B,D,E Pg.352
        timer_waitMillis(10);
        ADC0_ACTSS_R = ADC0_ACTSS_R & (~0x00000001); // ADC Sample Sequencer, disable ASEN0 SS0
        // ADC0_EMUX_R = (ADC0_EMUX_R & (~0x0000000F)); //ADC Event Multiplexer Select
        ADC0_SSMUX0_R = (ADC0_SSMUX0_R & (~0x0000000F)) | 0x0000000A; // Mux0 has Ain 10 as the first sample select
        timer_waitMillis(10);
        ADC0_SSCTL0_R = (ADC0_SSCTL0_R & (~0x0000000F))| 0x00000002; // no TS0 D0 IE0, yes END0
        //ADC0_SSPRI_R = (ADC0_SSPRI_R & (~0x00000003)); // Setting SS0 to highest priority Pg 841
        timer_waitMillis(10);
        ADC0_ACTSS_R = (ADC0_ACTSS_R & (~0x00000001)) | 0x00000001; // ADC Sample Sequencer, enables ASEN0 SS0
        timer_waitMillis(10);

    }
      //ADC Read

   int adc_read(void){
       int raw_data = 0;
       int total_dist = 0;
       //Sample IR sensor Output
       ADC0_PSSI_R |= 0x00000001; // Begins sampling on sample sequencer SS0 of ADC0_ACTSS_R
       raw_data = ADC0_SSFIFO0_R & 0xFFF;


       //total_dist = 5.84692 - 23001.5/raw_data + (6.7653*pow(10,7))/pow(raw_data,(2)); FIXME OLD LAB EQUATION
       total_dist = 5.51212 - 13846/raw_data + (5.3529*pow(10,7))/pow(raw_data,(2)) -1;

      //Read ADC conversion results & Display them
       return total_dist;
       //return raw_data;
  }


   float clean_IR_val(void){
       int i, j = 0;
       int val[8];
       float avg;
       int temp = 0;

       // fill array with data values
       for(i = 0; i<8; i++){
           val[i] = adc_read();

       }

       //Bubble Sort
       for(j = 0; j < 6; j++){
                  for(i = 0; i<7; i++){
                      if(val[i] > val[i+1]){
                          temp = val[i];
                          val[i] = val[i+1];
                          val[i+1] = temp;
                      }

                  }
             }

       // average middle vals
       avg = (val[1] + val[2] + val[3] + val[4] + val[5] + val[6]) /6.0;
       return avg;


   }



