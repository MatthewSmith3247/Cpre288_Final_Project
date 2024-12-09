/*
 * manual.c
 *
 *  Created on: Dec 7, 2024
 *      Author: jreiff
 */


//INCLUDE ALL THE INCLUDES
#include "adc.h" 
#include "uart.h" 
#include "Timer.h" 
#include "lcd.h" 
#include "math.h" 
#include "open_interface.h" 
#include "movement.h" 
#include "button.h" 
#include "ping.h" 
#include "servo.h" 
#include "adc.h" 
#include "methods.h" 
#include "audio.h" 
#include "IMU.h"
#include "manual.h"

//This file is for driving the robot in manual mode with zero autonomous functionality


void move_forward_detect(oi_t *sensor_data, int totalDistance)
{      
    float distanceTraveled = 0;
    float objOppDist; //The object distance from the bot to the object along the opposite side 
    char message[50];
    float angle_correction = 0; //the angle of the robot to keep it in a straight line 
    short forward_RWP = 95; // Right wheel power for forward movement
    short forward_LWP = 80; // Left wheel power for forward movement
    short turn_RWP = 50;    // Right wheel power for turning
    short turn_LWP = 50;    // Left wheel power for turning
    int clear = sensor_data->distance; // clear it out before the bot moves 
    char distMoved[50];
    
    //while the bot is moving forward it will stop if the cliff or bump signals are triggered
    while ((distanceTraveled < totalDistance) && (sensor_data->bumpRight == 0)
            && (sensor_data->bumpLeft == 0)
            && ((sensor_data->cliffFrontLeftSignal < 2600)
                    && (sensor_data->cliffFrontLeftSignal > 200))
            && ((sensor_data->cliffFrontRightSignal < 2600)
                    && (sensor_data->cliffFrontRightSignal > 200))
            && ((sensor_data->cliffLeftSignal < 2600)
                    && (sensor_data->cliffLeftSignal > 200))
            && ((sensor_data->cliffRightSignal < 2600)
                    && (sensor_data->cliffRightSignal > 200)))

    {
        /// ----This section handles keeping our wheels in a straight line while the bot is moving 

        angle_correction += sensor_data->angle; // Accumulate angle deviation

        // Adjust wheel power based on angle deviation
        if (angle_correction > DEGREE_ACCURACY)
        {
            forward_RWP -= 1; // Reduce right wheel power
            angle_correction = 0;
        }
        else if (angle_correction < -DEGREE_ACCURACY)
        {
            forward_RWP += 1; // Increase right wheel power
            angle_correction = 0;
        }
        oi_setWheels(forward_RWP, forward_LWP); // Set adjusted wheel speed
        ////--------------
        oi_update(sensor_data);
        distanceTraveled += sensor_data->distance; //sensor_data Distance Returns a Value in MM
        lcd_printf("Dist: %f", distanceTraveled);
    }
    //record the distance moved and send it to the GUI
    sprintf(distMoved, "moved\t%0.2f\n", distanceTraveled);
    cybot_send_string(distMoved);
    oi_setWheels(0, 0);

    if (sensor_data->bumpLeft == 1)
    {
        cybot_send_string("bump left\n");
        Audio_Specified_Song(0x00, Small_Obj_Hit);
    }

    else if (sensor_data->bumpRight == 1)
    {
        cybot_send_string("bump right\n");
        Audio_Specified_Song(0x00, Small_Obj_Hit);
    }
    if ((sensor_data->cliffLeftSignal >= 2600 || sensor_data->cliffFrontLeftSignal >= 2600)
            && (sensor_data->cliffRightSignal >= 2600
                    || sensor_data->cliffFrontRightSignal >= 2600))
    {
        Audio_Specified_Song(0x00, wall);
        cybot_send_string("wall both\n");
    }
    else if ((sensor_data->cliffLeftSignal <= 200
            || sensor_data->cliffFrontLeftSignal <= 200)
            && (sensor_data->cliffRightSignal <= 200
                    || sensor_data->cliffFrontRightSignal <= 200))
    {
        Audio_Specified_Song(0x00, Black_hole);
        cybot_send_string("hole both\n");
    }

    else if (sensor_data->cliffLeftSignal >= 2600
            || sensor_data->cliffFrontLeftSignal >= 2600)
    {
        Audio_Specified_Song(0x00, wall);
        cybot_send_string("wall left\n");

    }
    else if (sensor_data->cliffLeftSignal <= 200
            || sensor_data->cliffFrontLeftSignal <= 200)
    {
        Audio_Specified_Song(0x00, Black_hole);
        cybot_send_string("hole left\n");

    }

    else if (sensor_data->cliffRightSignal >= 2600
            || sensor_data->cliffFrontRightSignal >= 2600)
    {
        Audio_Specified_Song(0x00, wall);
        cybot_send_string("wall right\n");
    }
    else if (sensor_data->cliffRightSignal <= 200
            || sensor_data->cliffFrontRightSignal <= 200)
    {
        Audio_Specified_Song(0x00, Black_hole);
        cybot_send_string("hole right\n");
    }

}


//manual driver is the switch case that iterates over the different options of the keys that could be sent to the controller from the GUI

void manualDriver(oi_t *sensor_data)
{
    char message[50];
    char note[10];
    char buffer;
    int distance;
    int j, i;
    lcd_clear();
    oi_setWheels(0, 0);
    Audio_Specified_Song(0x00, Good_Morning);
    while (1)
    {
        i = 0;
        buffer = uart_receive();
        while (buffer != '\n')
        {
            message[i] = buffer;
            lcd_putc(message[i]);
            i++;
            buffer = uart_receive();
        }
        switch (message[0])
        {
        // switch only over the first index of the message
        case '1':
            //found bathroom
            Audio_Specified_Song(0x00, We_have_reached_the_Bathroom);
            cybot_send_string("Playing audio\n");
            break;
        case '2':
            //found kitchen
            Audio_Specified_Song(0x00, We_have_reached_the_Kitchen);
            cybot_send_string("Playing audio\n");
            break;
        case '3':
            //found Living Room -
            Audio_Specified_Song(0x00, We_have_reached_the_Living_Room);
            cybot_send_string("Playing audio\n");
            break;
        case '4':
            //find Exit --
            Audio_Specified_Song(0x00, We_have_reached_the_front_door);
            timer_waitMillis(6000);
            Audio_Specified_Song(0x00, end);
            cybot_send_string("Playing audio\n");
            break;
        case 'x':
            Audio_Specified_Song(0x00, Joke_1);
            cybot_send_string("Playing audio\n");
            break;
        case 'w':                       //Special Command with a 3 digit command
            cybot_send_string("forward amt\n");
            //receive amount to move forward -- assume 3 digits
            j = 0;
            buffer = uart_receive();
            while (buffer != '\n')
            {
                note[j] = buffer;
                lcd_putc(note[j]);
                j++;
                buffer = uart_receive();
            }
            //cybot_send_string("Received\n");
            lcd_clear();
            timer_waitMillis(50);
            distance = (note[0] - '0') * 100 + (note[1] - '0') * 10 + (note[2] - '0');
            lcd_printf("Dist: %d", distance);
            Audio_Specified_Song(0x00, Im_Moving_Fwd);
            move_forward_detect(sensor_data, (float)distance); 
            cybot_send_string("done Moving\n");
            lcd_clear();
            oi_setWheels(0, 0); // stop
            break;
        case 'a':
            turnCounterClockwise(sensor_data, 86.5);
            lcd_clear();
            lcd_printf("Turn CCW 90");
            oi_setWheels(0, 0); // stop
            break;
        case 'q':
            turnCounterClockwise(sensor_data, 43.245);
            lcd_clear();
            oi_setWheels(0, 0); // stop
            break;
        case 'd':
            turnClockwise(sensor_data, 86.5); // was 90 before
            lcd_clear();
            oi_setWheels(0, 0); // stop
            break;
        case 'e':
            turnClockwise(sensor_data, 43.245);
            lcd_clear();
            oi_setWheels(0, 0); // stop
            break;
        case 'm': // is sent for scan
            fullScanManual(0, 180);
            break;
        case 'f': // is sent for scan
            fastScanManual(0, 180);
            break;

        case 'g': // is sent for scan
            fullScanManualNoPlot(0, 180);
            break;

        case 's':
            cybot_send_string("backward amt\n");
            //receive amount to move forward -- assume 3 digits
            j = 0;
            buffer = uart_receive();
            while (buffer != '\n')
            {
                note[j] = buffer;
                lcd_putc(note[j]);
                j++;
                buffer = uart_receive();
            }
            //cybot_send_string("Received\n");
            lcd_clear();
            timer_waitMillis(50);
            distance = (note[0] - '0') * 100 + (note[1] - '0') * 10
                    + (note[2] - '0') + 50;
            moveBackward(sensor_data, (float)distance);
            lcd_clear();
            oi_setWheels(0, 0); // stop
            break;
        case 'p':
            oi_setWheels(0, 0);
            oi_free(sensor_data);
            break;
        default:
            cybot_send_string("Enter valid command\n");
            break;
        }

    }
}


int fastScanManual(int startDeg, int endDeg)
{
//will scan at increment of 2 degrees. we can change this to be larger as we see fit.
// Scans within 5 sec, a little less accurate, but the speed is useful 
//scan variable declaration
    int object_count;
    float object_distance[5];
    float objOppDist;
    int initial_angle[5];
    int midpoint_angle[5];
    int final_angle[5];
    char scanned_data[45];
    float scanned_distance_vals_PING[4];
    float scanned_distance_vals_IR[4];
    float val[5], temp;
    int j;
    int scanned_degree_vals[4];
    int alreadyRec = 0;
    int pingDist;

// Variables for object count 
    int heading = 0;
    float delta_distance;
    float initial_dist[5];
    float final_dist[5];
    float linear_distance[5];
    int i;

    //heading = (int)((float)read_euler_heading(0x29) / 16.0);
    object_count = 0;
    alreadyRec = 0; // reset to 0 before scan
    oi_setWheels(0, 0);
    int curr_degree = startDeg;
//cybotSendString("Degrees\t\tDistance IR\n");
    if (curr_degree == 0)
    {
        servo_move(0);
        timer_waitMillis(300); // FIX ME
        // Hard set point 1 of scan
        servo_move(0);
        scanned_distance_vals_IR[0] = 250; // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[0] = 0; // take account for what degree you are located at
        servo_move(2);
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = 250; // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[1] = 2; // take account for what degree you are located at
        curr_degree = 4;
    }
    else
    {
        servo_move(startDeg);
        timer_waitMillis(300);
        // Hard set point 1 of scan
        servo_move(startDeg);
        scanned_distance_vals_IR[0] = clean_IR_val(); // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[0] = startDeg; // take account for what degree you are located at
        servo_move((startDeg + 2));
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = clean_IR_val(); // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[1] = startDeg + 2; // take account for what degree you are located at
        curr_degree = startDeg + 4;

    }
// collect data
    while (curr_degree <= endDeg)
    {
        servo_move(curr_degree); // scanning
        scanned_distance_vals_IR[2] = clean_IR_val();
       if (scanned_distance_vals_IR[2] > 75)
        {
            scanned_distance_vals_IR[2] = 250;
        }

        // take account for what degree you are located at
       scanned_distance_vals_PING[2] = ping_read(); // find the distance away from IR sensor_data at given degree for PING
        scanned_degree_vals[2] = curr_degree;

        //Clean IR
        if (scanned_distance_vals_IR[0] < (scanned_distance_vals_IR[2] + 20)
                && scanned_distance_vals_IR[0]
                        > (scanned_distance_vals_IR[2] - 20))
        {
            scanned_distance_vals_IR[1] = (scanned_distance_vals_IR[0]
                    + scanned_distance_vals_IR[2]) / 2;
        }


        // Find Objects and widths
        if (object_count < 5 && curr_degree > 4)
        {
            delta_distance = scanned_distance_vals_IR[1]
                    - scanned_distance_vals_IR[0];
            //alreadyRec is an int for seeing if you are already recording, to prevent two objects at the same degree
            if (delta_distance <= -30 && alreadyRec == 0 && scanned_distance_vals_PING[2] < 75)
            { // falling edge, if there is a change in distance that is drastic
                alreadyRec = 1;
                initial_angle[object_count] = scanned_degree_vals[0]; // take initial angle measurement
                initial_dist[object_count] = scanned_distance_vals_IR[0]; //record that new radius away ///FIX ME CHANGED SCNDDIST_IR from 0 to 1

            }
            else if (delta_distance >= 30 && alreadyRec == 1)
            { //rising edge, if there is a change in distance that is drastic
                final_angle[object_count] = scanned_degree_vals[0]; // record the final angle of the object
                final_dist[object_count] = scanned_distance_vals_IR[0];
                if (final_angle[object_count] - initial_angle[object_count] <= 8
                        || final_angle[object_count]
                                - initial_angle[object_count] >= 75)
                {

                }
                else
                {
                    object_count++; // record new object
                    alreadyRec = 0;
                }

            }
        }

        //send data to UART FIXME Debugging
        sprintf(scanned_data, "%d\t\t%.0f\t\n\r", scanned_degree_vals[0],
                scanned_distance_vals_IR[0]);
      //  cybot_send_string(scanned_data);
        // rotate values to the left 1 spot in the array
          //Ping
          scanned_distance_vals_PING[0] = scanned_distance_vals_PING[1];
          scanned_distance_vals_PING[1] = scanned_distance_vals_PING[2];
          // IR
          scanned_distance_vals_IR[0] = scanned_distance_vals_IR[1];
          scanned_distance_vals_IR[1] = scanned_distance_vals_IR[2];
          //Degrees
          scanned_degree_vals[0] = scanned_degree_vals[1];
          scanned_degree_vals[1] = scanned_degree_vals[2];

          curr_degree = curr_degree + 2; //Increment to collect data every 4 degrees FIXME
          timer_waitMillis(10);
      }
    //Scan completed -- Data should be finished

    // Find Distances of Objects and store the values
        for (i = 0; i < object_count; i++)
        {
            // Find midpoint of angles
            midpoint_angle[i] = (final_angle[i] - initial_angle[i]) / 2
                    + initial_angle[i];
            // Take reading at midpoint
            servo_move(midpoint_angle[i]);
            timer_waitMillis(1000);
            servo_move(midpoint_angle[i]);
            // store PING value from reading
            object_distance[i] = ping_read();
        }
        //Clear out the array
            scanned_data[0] = '\0';
        //Send Objects and distances to GUI
            for (i = 0; i < object_count; i++)
            {
                objOppDist = object_distance[i]
                        * sin((abs(90 - midpoint_angle[i])) * M_PI / 180);
                //Format Data to be sent
                //Distance from bot \t angle in perspective to bot
                sprintf(scanned_data, "%.2f\t%d\t%.2f\n", object_distance[i],
                        midpoint_angle[i], objOppDist);
                cybot_send_string(scanned_data);
            }
        // tell GUI there are no more objects
            cybot_send_string("finished\n");

            return 1;
        }

int fullScanManualNoPlot(int startDeg, int endDeg)
{
//will scan at increment of 2 degrees. we can change this to be larger as we see fit.
//Scans with a bubble sort for a more precise scan, takes a little longer
//scan variable declaration
    int object_count;
    float object_distance[5];
    float objOppDist;
    int initial_angle[5];
    int midpoint_angle[5];
    int final_angle[5];
    char scanned_data[45];
    float scanned_distance_vals_PING[4];
    float scanned_distance_vals_IR[4];
    float val[5], temp;
    int j;
    int scanned_degree_vals[4];
    int alreadyRec = 0;
    int pingDist;

// Variables for object count FIXME move to correct scope (m)
    int heading = 0;
    float delta_distance;
    float initial_dist[5];
    float final_dist[5];
    float linear_distance[5];
    int i;

    //heading = (int)((float)read_euler_heading(0x29) / 16.0);
    object_count = 0;
    alreadyRec = 0; // reset to 0 before scan
    oi_setWheels(0, 0);
    int curr_degree = startDeg;
//cybotSendString("Degrees\t\tDistance IR\n");
    if (curr_degree == 0)
    {
        servo_move(0);
        timer_waitMillis(300); // FIX ME
        // Hard set point 1 of scan
        servo_move(0);
        scanned_distance_vals_IR[0] = 250; // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[0] = 0; // take account for what degree you are located at
        servo_move(2);
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = 250; // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[1] = 2; // take account for what degree you are located at
        curr_degree = 4;
    }
    else
    {
        servo_move(startDeg);
        timer_waitMillis(300);
        // Hard set point 1 of scan
        servo_move(startDeg);
        scanned_distance_vals_IR[0] = clean_IR_val(); // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[0] = startDeg; // take account for what degree you are located at
        servo_move((startDeg + 2));
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = clean_IR_val(); // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[1] = startDeg + 2; // take account for what degree you are located at
        curr_degree = startDeg + 4;

    }
// collect data
    while (curr_degree <= endDeg)
    {
        servo_move(curr_degree); // scanning
        // take in all values for current scan
        // take in 3 values at current angle and translate to cm
        //timer_waitMillis(10);
        for (i = 0; i < 5; i++)
        {
            val[i] = clean_IR_val();
            timer_waitMillis(10);
        }
        // bubble sort to get outliers at index 0 and index 4
        for (j = 0; j < 3; j++)
        {
            for (i = 0; i < 4; i++)
            {
                if (val[i] > val[i + 1])
                {
                    temp = val[i];
                    val[i] = val[i + 1];
                    val[i + 1] = temp;
                }

            }
        }
        // average the three values and store
        scanned_distance_vals_IR[2] = (val[1] + val[2] + val[3]) / 3;

        //scanned_distance_vals_IR[2] = clean_IR_val();
       if (scanned_distance_vals_IR[2] > 75)
        {
            scanned_distance_vals_IR[2] = 250;
        }

        // take account for what degree you are located at
       scanned_distance_vals_PING[2] = ping_read(); // find the distance away from IR sensor_data at given degree for PING
        scanned_degree_vals[2] = curr_degree;

        //Clean IR
        if (scanned_distance_vals_IR[0] < (scanned_distance_vals_IR[2] + 20)
                && scanned_distance_vals_IR[0]
                        > (scanned_distance_vals_IR[2] - 20))
        {
            scanned_distance_vals_IR[1] = (scanned_distance_vals_IR[0]
                    + scanned_distance_vals_IR[2]) / 2;
        }


        // Find Objects and widths
        if (object_count < 5 && curr_degree > 4)
        {
            delta_distance = scanned_distance_vals_IR[1]
                    - scanned_distance_vals_IR[0];
            //alreadyRec is an int for seeing if you are already recording, to prevent two objects at the same degree
            if (delta_distance <= -30 && alreadyRec == 0 && scanned_distance_vals_PING[2] < 75)
            { // falling edge, if there is a change in distance that is drastic
                alreadyRec = 1;
                initial_angle[object_count] = scanned_degree_vals[0]; // take initial angle measurement
                initial_dist[object_count] = scanned_distance_vals_IR[0]; //record that new radius away ///FIX ME CHANGED SCNDDIST_IR from 0 to 1

            }
            else if (delta_distance >= 30 && alreadyRec == 1)
            { //rising edge, if there is a change in distance that is drastic
                final_angle[object_count] = scanned_degree_vals[0]; // record the final angle of the object
                final_dist[object_count] = scanned_distance_vals_IR[0];
                if (final_angle[object_count] - initial_angle[object_count] <= 8
                        || final_angle[object_count]
                                - initial_angle[object_count] >= 75)
                {

                }
                else
                {
                    object_count++; // record new object
                    alreadyRec = 0;
                }

            }
        }

        //send data to UART FIXME Debugging
        sprintf(scanned_data, "%d\t\t%.0f\t\n\r", scanned_degree_vals[0],
                scanned_distance_vals_IR[0]);
      //  cybot_send_string(scanned_data);
        // rotate values to the left 1 spot in the array
          //Ping
          scanned_distance_vals_PING[0] = scanned_distance_vals_PING[1];
          scanned_distance_vals_PING[1] = scanned_distance_vals_PING[2];
          // IR
          scanned_distance_vals_IR[0] = scanned_distance_vals_IR[1];
          scanned_distance_vals_IR[1] = scanned_distance_vals_IR[2];
          //Degrees
          scanned_degree_vals[0] = scanned_degree_vals[1];
          scanned_degree_vals[1] = scanned_degree_vals[2];

          curr_degree = curr_degree + 2; //Increment to collect data every 4 degrees FIXME
          timer_waitMillis(10);
      }
    //Scan completed -- Data should be finished

    // Find Distances of Objects and store the values
        for (i = 0; i < object_count; i++)
        {
            // Find midpoint of angles
            midpoint_angle[i] = (final_angle[i] - initial_angle[i]) / 2
                    + initial_angle[i];
            // Take reading at midpoint
            servo_move(midpoint_angle[i]);
            timer_waitMillis(1000);
            servo_move(midpoint_angle[i]);
            // store PING value from reading
            object_distance[i] = ping_read();
        }
        //Clear out the array
            scanned_data[0] = '\0';
        //Send Objects and distances to GUI
            for (i = 0; i < object_count; i++)
            {
                objOppDist = object_distance[i]
                        * sin((abs(90 - midpoint_angle[i])) * M_PI / 180);
                //Format Data to be sent
                //Distance from bot \t angle in perspective to bot
                sprintf(scanned_data, "%.2f\t%d\t%.2f\n", object_distance[i],
                        midpoint_angle[i], objOppDist);
                cybot_send_string(scanned_data);
            }
        // tell GUI there are no more objects
            cybot_send_string("finished\n");

            return 1;
        }

int fullScanManual(int startDeg, int endDeg)
{
//will scan at increment of 2 degrees. we can change this to be larger as we see fit.
    //Includes a bubble sort and multiple scans at each point for accurate data. It sends the data points to the GUI to be plotted in a 2-D polar coordinate system
//scan variable declaration
    int object_count;
    float object_distance[5];
    int initial_angle[5];
    int midpoint_angle[5];
    int final_angle[5];
    char scanned_data[45];
    float scanned_distance_vals_PING[4];
    float scanned_distance_vals_IR[4];
    float val[5], temp;
    int j;
    int scanned_degree_vals[4];
    int alreadyRec = 0;
    int pingDist;
    int objOppDist;
// Variables for object count FIXME move to correct scope (m)
    //heading = (int)((float)read_euler_heading(0x29) / 16.0);

    float delta_distance;
    float initial_dist[5];
    float final_dist[5];
    float linear_distance[5];
    int i;

    object_count = 0;
    alreadyRec = 0; // reset to 0 before scan
    oi_setWheels(0, 0);
    int curr_degree = startDeg;
//cybotSendString("Degrees\t\tDistance IR\n");
    if (curr_degree == 0)
    {
        servo_move(0);
        timer_waitMillis(300); // FIX ME
        // Hard set point 1 of scan
        servo_move(0);
        scanned_distance_vals_IR[0] = 250; // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[0] = 0; // take account for what degree you are located at
        servo_move(2);
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = 250; // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[1] = 2; // take account for what degree you are located at
        curr_degree = 4;
    }
    else
    {
        servo_move(startDeg);
        timer_waitMillis(300);
        // Hard set point 1 of scan
        servo_move(startDeg);
        scanned_distance_vals_IR[0] = clean_IR_val(); // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[0] = startDeg; // take account for what degree you are located at
        servo_move((startDeg + 2));
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = clean_IR_val(); // find the distance away from IR sensor_data at given degree for IR and translate it
        scanned_degree_vals[1] = startDeg + 2; // take account for what degree you are located at
        curr_degree = startDeg + 4;

    }
// collect data
    while (curr_degree <= endDeg)
    {
        servo_move(curr_degree); // scanning
        // take in all values for current scan
        scanned_distance_vals_PING[2] = ping_read(); // find the distance away from IR sensor_data at given degree for PING
        // take in 3 values at current angle and translate to cm
        //timer_waitMillis(10);
        for (i = 0; i < 5; i++)
        {
            val[i] = clean_IR_val();
            timer_waitMillis(10);
        }
//        // bubble sort to get outliers at index 0 and index 4
        for (j = 0; j < 5; j++)
        {
            for (i = 0; i < 4; i++)
            {
                if (val[i] > val[i + 1])
                {
                    temp = val[i];
                    val[i] = val[i + 1];
                    val[i + 1] = temp;
                }

            }
        }
        // average the three values and store
        scanned_distance_vals_IR[2] = (val[1] + val[2] + val[3]) / 3;

       if (scanned_distance_vals_IR[2] > 75)
        {
            scanned_distance_vals_IR[2] = 250;
        }

        // take account for what degree you are located at

        scanned_degree_vals[2] = curr_degree;

        //Clean IR
        if (scanned_distance_vals_IR[0] < (scanned_distance_vals_IR[2] + 20)
                && scanned_distance_vals_IR[0]
                        > (scanned_distance_vals_IR[2] - 20))
        {
            scanned_distance_vals_IR[1] = (scanned_distance_vals_IR[0]
                    + scanned_distance_vals_IR[2]) / 2;
        }
        // Take PING to check distance
        pingDist = scanned_distance_vals_PING[2];

        // Find Objects and widths
        if (object_count < 5 && curr_degree > 4)
        {
            delta_distance = scanned_distance_vals_IR[1]
                    - scanned_distance_vals_IR[0];
            //alreadyRec is an int for seeing if you are already recording, to prevent two objects at the same degree
            if (delta_distance <= -30 && alreadyRec == 0 && pingDist < 75)
            { // falling edge, if there is a change in distance that is drastic
                alreadyRec = 1;
                initial_angle[object_count] = scanned_degree_vals[0]; // take initial angle measurement
                initial_dist[object_count] = scanned_distance_vals_IR[0]; //record that new radius away ///FIX ME CHANGED SCNDDIST_IR from 0 to 1

            }
            else if (delta_distance >= 30 && alreadyRec == 1)
            { //rising edge, if there is a change in distance that is drastic
                final_angle[object_count] = scanned_degree_vals[0]; // record the final angle of the object
                final_dist[object_count] = scanned_distance_vals_IR[0];
                if (final_angle[object_count] - initial_angle[object_count] <= 8
                        || final_angle[object_count]
                                - initial_angle[object_count] >= 75)
                {

                }
                else
                {
                    object_count++; // record new object
                    alreadyRec = 0;
                }

            }
        }

        //send data to UART FIXME Debugging
        sprintf(scanned_data, "%d\t\t%.0f\t\n\r", scanned_degree_vals[0],
                scanned_distance_vals_IR[0]);
        cybot_send_string(scanned_data);
        // rotate values to the left 1 spot in the array
          //Ping
          scanned_distance_vals_PING[0] = scanned_distance_vals_PING[1];
          scanned_distance_vals_PING[1] = scanned_distance_vals_PING[2];
          // IR
          scanned_distance_vals_IR[0] = scanned_distance_vals_IR[1];
          scanned_distance_vals_IR[1] = scanned_distance_vals_IR[2];
          //Degrees
          scanned_degree_vals[0] = scanned_degree_vals[1];
          scanned_degree_vals[1] = scanned_degree_vals[2];

          curr_degree = curr_degree + 2; //Increment to collect data every 4 degrees FIXME
          timer_waitMillis(10);
      }
    //Scan completed -- Data should be finished
    // send last 2 values FIXME Debugging
        sprintf(scanned_data, "%d\t\t%.0f\n\r", scanned_degree_vals[1],
                scanned_distance_vals_IR[1]);
    cybot_send_string(scanned_data);
        sprintf(scanned_data, "%d\t\t%.0f\n\r", scanned_degree_vals[2],
                scanned_distance_vals_IR[2]);
    cybot_send_string(scanned_data);

    // Find Distances of Objects and store the values
        for (i = 0; i < object_count; i++)
        {
            // Find midpoint of angles
            midpoint_angle[i] = (final_angle[i] - initial_angle[i]) / 2
                    + initial_angle[i];
            // Take reading at midpoint
            servo_move(midpoint_angle[i]);
            timer_waitMillis(1000);
            servo_move(midpoint_angle[i]);
            // store PING value from reading
            object_distance[i] = ping_read();
        }
        //Clear out the array
            scanned_data[0] = '\0';
        //Send Objects and distances to GUI
            for (i = 0; i < object_count; i++)
            {
                objOppDist = object_distance[i]
                        * sin((abs(90 - midpoint_angle[i])) * M_PI / 180);
                //Format Data to be sent
                //Distance from bot \t angle in perspective to bot
                sprintf(scanned_data, "%.2f\t%d\t%d\n", object_distance[i],
                        midpoint_angle[i], objOppDist);
                cybot_send_string(scanned_data);
            }
        // tell GUI there are no more objects
            cybot_send_string("finished\n");

            return 1;
        }
