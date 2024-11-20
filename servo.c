/*
 * servo.c
 *
 *  Created on: Nov 3, 2024
 *      Author: mss3247
 */
#include "servo.h"


//A button was pushed to generate an interrupt aka a flag
extern volatile int button_event;
//Which button was pushed when an interrupt occurs
extern volatile int button_num;
//calculated value for cycles per 1 degree (How much the pulse width should change between degrees)
volatile int cycles_per_degree = 155;
volatile int right_calibration_value = 336360;
volatile int left_calibration_value = 308480;

 void servo_init (void){

         //Initializing Clocks
         SYSCTL_RCGCTIMER_R |= 0x02; // Enable Timer 1
         SYSCTL_RCGCGPIO_R |= 0b010; //Enable Clock for Port B
         //Wait for clocks to actually enable
         timer_waitMillis(10);

    //Configure PB5
         //Direction should be input
         GPIO_PORTB_DIR_R |= 0x20;
         //Digitally Enable Pin 5
         GPIO_PORTB_DEN_R |= 0x20;
         //Turn on the AFSEL for Pin 5
         GPIO_PORTB_AFSEL_R |= 0x20;
         //Clear the previous PCTL for Pin 5
         GPIO_PORTB_PCTL_R &= ~(0x0F00000);
         //Set port control to use the 7th column which is Timer 1B
         GPIO_PORTB_PCTL_R |= 0x0700000;

     //A timer is configured to PWM mode using the following sequence: (Can be found on Pg.725 on Tiva Datasheet)

        // 1. Ensure the timer is disabled (the TnEN bit is cleared) before making any changes
             //Disabling timer by forcing Bit 8 to 0
            TIMER1_CTL_R &= ~(0x100);

       // 2. Write 0x04 to GPTM Config to Configure timer to 16 bit "split" mode
            //Force zeros in the first 3 bits
            TIMER1_CFG_R &= ~(0b111);
            //Place the 0x04 for the split mode
            TIMER1_CFG_R |= 0x4;
      // 3. In Timer Mode set Bit 3&1 to 1 set Bit 2&0 to 0
            //Force 0 to Bit 2 and 0
            TIMER1_TBMR_R &= (~0b101);
            //Force 1 to Bit 3 & 1
            TIMER1_TBMR_R |= 0b1010;

      // 4. Set Bit 14 in TIMER_CTL to 0, so it is not inverted
            TIMER1_CTL_R &= ~(0x4000);

      // 5.  Set Prescalar

             int period = 344000;
             int msb = period >> 16;
             int lsb = period & 0xFFFF;


            //Configure Prescaler Register
            TIMER1_TBPR_R &= ~(0xFF);
            TIMER1_TBPR_R |= (msb & 0xFF);


      // 7. Load Period into Timer Interval Load
            TIMER1_TBILR_R &= ~(0xFFFF);

            TIMER1_TBILR_R |= (lsb & 0xFFFF);

      // 8. Load Match Register with Period-PulseWidth
            //tested values here until I found the correct ones, these numbers are how I calculated the cycles per degree number ([actual0 - actual180] / 180)
            int inverse_pulse_width = right_calibration_value; // Actual 0
            //int inverse_pulse_width = 308500; // Actual 180


                // need to set into a prescaler (Number to big)
                int msb2 = inverse_pulse_width >> 16;
                int lsb2 = inverse_pulse_width & 0xFFFF;

                TIMER1_TBPMR_R &= ~(0xFF); // clear any value
                TIMER1_TBPMR_R |= (msb2 & 0xFF); // Set value to msb

                // Load in LSB
                TIMER1_TBMATCHR_R &= ~(0xFFFF);
                TIMER1_TBMATCHR_R |= (lsb2 & 0xFFFF);


      // 9. Re enable the Timer
            TIMER1_CTL_R |= (0x100);
//
 }





 void servo_move(int servo_degree){
     //Variables for the function
     int temp, msb,lsb;



     // Make sure that degree is within our valid range
     if (servo_degree < 0){
         servo_degree = 0;
     }
     if (servo_degree > 180){
         servo_degree = 180;
     }
     //find cycles_per_degree
     //Do calibration calculation to find the slope of the line
     int cycles_per_degree = (right_calibration_value - left_calibration_value) / (-180);


     //calculate the match register value
     temp = servo_degree * cycles_per_degree + right_calibration_value;

     // Disable Timer
     TIMER1_CTL_R &= ~(0x100);

     // Load in LSB
              //set match to zero
              TIMER1_TBMATCHR_R &= ~(0xFFFF);
              TIMER1_TBPMR_R &= ~(0xFF);
              //Bitwise operations to pass the number into the match register and the prescalar
              msb = temp >> 16;
              lsb = temp & 0xFFFF;
              TIMER1_TBPMR_R |= (msb & 0xFF);
              TIMER1_TBMATCHR_R |= (lsb & 0xFFFF);

      // Re enable the Timer
     TIMER1_CTL_R |= (0x100);

 }



 void servo_calibrate(){
     int done = 0;
     lcd_printf("Calibration");
     // variables for bitwise operations
     int temp, msb,lsb = 0;
     // //Do calibration calculation
     int cycles_per_degree = (right_calibration_value - left_calibration_value) / 180;

     while (1){
        if(button_event){
            button_event = 0;
            switch (button_num) {

                case 1: // move Counter Clockwise
                    TIMER1_CTL_R &= ~(0x100);

                    // Load in LSB
                        //store current value in match register & prescalar
                         temp = (TIMER1_TBPMR_R<< 16) | TIMER1_TBMATCHR_R & 0xFFFF;
                        //set match to zero
                        TIMER1_TBMATCHR_R &= ~(0xFFFF);
                        TIMER1_TBPMR_R &= ~(0xFF);
                        //reset match to the original - degree to move  counter clockwise
                        temp = temp - (5*cycles_per_degree);
                        msb = temp >> 16;
                        lsb = temp & 0xFFFF;
                        TIMER1_TBPMR_R |= (msb & 0xFF);
                        TIMER1_TBMATCHR_R |= (lsb & 0xFFFF);
                        // Re enable the Timer
                        TIMER1_CTL_R |= (0x100);
                   break;

                case 2:  // move clockwise
                    // Disable Timer
                     TIMER1_CTL_R &= ~(0x100);
                     // Load in LSB
                         //store current value in match register & prescalar
                          temp = (TIMER1_TBPMR_R<< 16) | TIMER1_TBMATCHR_R & 0xFFFF;
                         //set match to zero
                         TIMER1_TBMATCHR_R &= ~(0xFFFF);
                         TIMER1_TBPMR_R &= ~(0xFF);
                         //reset match to the original + degree to move  clockwise
                         temp = temp + (5*cycles_per_degree);
                         msb = temp >> 16;
                         lsb = temp & 0xFFFF;
                         TIMER1_TBPMR_R |= (msb & 0xFF);
                         TIMER1_TBMATCHR_R |= (lsb & 0xFFFF);
                       // Re enable the Timer
                        TIMER1_CTL_R |= (0x100);
                   break;

                case 3:  //Save Value from Match Register for right calibration value
                    right_calibration_value = (TIMER1_TBPMR_R << 16) | TIMER1_TBMATCHR_R & 0xFFFF;
                    lcd_printf("%d", right_calibration_value);
                   break;

                case 4:  //Save Value from Match Register for left calibration value
                    left_calibration_value =  (TIMER1_TBPMR_R << 16) | TIMER1_TBMATCHR_R & 0xFFFF;
                    lcd_printf("%d", left_calibration_value);
                    done  = 1;
                   break;

            }

        }
        if (done){
            break;
        }
     }


 }
