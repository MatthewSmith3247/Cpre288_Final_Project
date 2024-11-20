//INCLUDE ALL THE INCLUDES
#include "adc.h" //YES
#include "uart.h" //YES
#include "Timer.h" //YES
#include "lcd.h" //YES
#include "math.h" //JUST MATH SO YES
#include "open_interface.h" //YES
#include "movement.h" //YES
#include "button.h" //YES
#include "ping.h" //YES
#include "servo.h" //NO? THEY DO IT DIFFERENTLY
#include "adc.h" //YES
#include "methods.h" //YES
#include "audio.h" //YES
#include "cyBot_uart.h"

/*
void cybotSendString(char string[50])
{
    int j;
    for (j = 0; j <= strlen(string); j++)
    {
        uart_sendChar(string[j]);
    }
}
*/

void sendToPutty(char string[], int length)
{

    int i = 0;

    for (i = 0; i < length; i++)
    {
        cyBot_sendByte(string[i]);
    }
}

void printIR(oi_t *sensor)
{
    char array[10];
    oi_update(sensor);
    sprintf(array, "%d\n", sensor->cliffFrontLeftSignal); // Format float to string
    sendToPutty(array, strlen(array));
}

void moveForwardDetect(oi_t *sensor, int totalDistance)
{
    /*
     * Move forward as long as there isn't a low object or out of bounds tape | (bumpSensor == 0) && (cliffSensor == 0)
     * Also store the distance we've traveled and subtract that from the total distance then recursively call moveForwardDetect(sensor, newDistance)
     * If IR value is greater than 1800 or less than 1200 based on testing.
     */

    double distanceTraveled = 0;
    double remainingDistance = 0;

    while ((distanceTraveled < totalDistance) && (sensor->bumpRight == 0)
            && (sensor->bumpLeft == 0)
            && ((sensor->cliffFrontLeftSignal < 2400)
                    && (sensor->cliffFrontLeftSignal > 100))
            && ((sensor->cliffFrontRightSignal < 2400)
                    && (sensor->cliffFrontRightSignal > 100))
            && ((sensor->cliffLeftSignal < 2400)
                    && (sensor->cliffLeftSignal > 100))
            && ((sensor->cliffRightSignal < 2400)
                    && (sensor->cliffRightSignal > 100)))
    {
        oi_setWheels(100, 100);
        oi_update(sensor);
        distanceTraveled += sensor->distance;
    }

    oi_setWheels(0, 0);
    remainingDistance = totalDistance - distanceTraveled;

    if (sensor->bumpLeft == 1)
    {
        moveBackward(sensor, 75);
        turnClockwise(sensor, 88); //CHANGE BASED ON TESTING FIXME  //88
        moveForward(sensor, 75); //CHANGE BASED ON TESTING
        turnCounterClockwise(sensor, 88); //CHANGE BASED ON TESTING  //88
        moveForwardDetect(sensor, (remainingDistance + 75));
    }

    else if (sensor->bumpRight == 1)
    {
        moveBackward(sensor, 75);
        turnCounterClockwise(sensor, 88); //88
        moveForward(sensor, 75); //CHANGE BASED ON TESTING FIXME
        turnClockwise(sensor, 88); //CHANGE BASED ON TESTING  //88
        moveForwardDetect(sensor, (remainingDistance + 75));
    }

    if (sensor->cliffLeftSignal >= 2400 || sensor->cliffLeftSignal <= 100)
    {
        //IDK HOW TO REALLY HANDLE THIS LOL
        turnClockwise(sensor, 178); // 178
        moveForwardDetect(sensor, (remainingDistance));
    }

    else if (sensor->cliffRightSignal >= 2400 || sensor->cliffRightSignal <= 100)
    {
        //IDK HOW TO REALLY HANDLE THIS LOL
        turnCounterClockwise(sensor, 178); //
        moveForwardDetect(sensor, (remainingDistance));
    }

}

int objectCollision()
{
    //RETURN 1 FOR TRUE 0 FOR FALSE
    //this should be used for detecting the edge boundary, hole and small objects and return what object it ran into.

    //cliffSensor is in oi_parsePacket(); pg 24 of open interface spec. in resources file of 288 website.
    return 0;
}

int fastScan(int startDeg, int endDeg)
{
    //will scan at increment of 4 degrees. we can change this to be larger as we see fit.
    //scan variable declaration

    char scanned_data[45];
    float scanned_distance_vals_PING[4];
    float scanned_distance_vals_IR[4];
    float val[5], temp;
    int j;
    int scanned_degree_vals[4], midpoint_angle;
    int alreadyRec = 0;
    float min;
    int min_inital_angle, min_final_angle;

    // Variables for object count FIXME move to correct scope (m)
    int object_count = 0;
    float object_distance[5], delta_distance; // lets try not to hard code a finite amount of these objects to make future labs easier
    int initial_angle[5]; // lets try not to hard code a finite amount of these objects to make future labs easier
    int final_angle[5];
    float initial_dist[5];
    float final_dist[5];
    float linear_distance[5];
    // cyBOT_Scan_t collected_data;

    //initialize for IR

    //char buffer[10];
    object_count = 0;
    alreadyRec = 0; // reset to 0 before scan
    oi_setWheels(0, 0);
    int i = 2;
    int curr_degree = startDeg;
    //cybotSendString("Degrees\t\tDistance IR\n");
    if (curr_degree == 0)
    {
        servo_move(0);
        timer_waitMillis(300); // FIX ME
        // Hard set point 1 of scan
        servo_move(0);
        scanned_distance_vals_IR[0] = 250; // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[0] = 0; // take account for what degree you are located at
        servo_move(2);
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = 250; // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[1] = 2; // take account for what degree you are located at
        curr_degree = 4;
    }
    else
    {
        servo_move(startDeg);
        timer_waitMillis(300);
        // Hard set point 1 of scan
        servo_move(startDeg);
        scanned_distance_vals_IR[0] = clean_IR_val(); // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[0] = startDeg; // take account for what degree you are located at
        servo_move((startDeg + 2));
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = clean_IR_val(); // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[1] = startDeg + 2; // take account for what degree you are located at
        curr_degree = startDeg + 4;

    }
    // collect data
    while (curr_degree <= endDeg)
    {
        servo_move(curr_degree); // scanning
        // take in all values for current scan
        scanned_distance_vals_PING[2] = ping_read(); // find the distance away from IR sensor at given degree for PING
        // take in 3 values at current angle and translate to cm
        timer_waitMillis(10);
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

        if (scanned_distance_vals_IR[2] > 100)
        {
            scanned_distance_vals_IR[2] = 250;
        }

        // take account for what degree you are located at
        scanned_degree_vals[2] = curr_degree;

        // clean middle piece of data i.e data in scanned_distance_vals[1]
        //clean PING
        //      if ( scanned_distance_vals_PING[0] < (scanned_distance_vals_PING[2] + 20) && scanned_distance_vals_PING[0] > (scanned_distance_vals_PING[2] - 20)){
        //                  scanned_distance_vals_PING[1] = (scanned_distance_vals_PING[0] + scanned_distance_vals_PING[2]) / 2; // Matt is paranoid............
        //      }
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
            if (delta_distance <= -30 && alreadyRec == 0)
            { // falling edge, if there is a change in distance that is drastic
                alreadyRec = 1;
                initial_angle[object_count] = scanned_degree_vals[0]; // take initial angle measurement
                initial_dist[object_count] = scanned_distance_vals_IR[1]; //record that new radius away ///FIX ME CHANGED SCNDDIST_IR from 0 to 1

            }
            else if (delta_distance >= 30 && alreadyRec == 1)
            { //rising edge, if there is a change in distance that is drastic
                final_angle[object_count] = scanned_degree_vals[0]; // record the final angle of the object
                final_dist[object_count] = scanned_distance_vals_IR[1];
                if (final_angle[object_count] - initial_angle[object_count] <= 2
                        || final_angle[object_count]
                                - initial_angle[object_count] >= 75)
                { ///FIX ME CHANGED 6 to 2 in angle count

                }
                else
                {
                    object_count++; // record new object
                    alreadyRec = 0;
                }

            }
        }
        //send data to UART
        sprintf(scanned_data, "%d\t\t%.0f\t\n\r", scanned_degree_vals[0],
                scanned_distance_vals_IR[0]);
        //cybotSendString(scanned_data);

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

        curr_degree = curr_degree + 4; //Increment to collect data every 2 degrees FIXME
        timer_waitMillis(10);

    }
    // send last 2 values
    sprintf(scanned_data, "%d\t\t%.0f\n\r", scanned_degree_vals[1],
            scanned_distance_vals_IR[1]);
    //cybotSendString(scanned_data);
    sprintf(scanned_data, "%d\t\t%.0f\n\r", scanned_degree_vals[2],
            scanned_distance_vals_IR[2]);
    //cybotSendString(scanned_data);

    // PING to Center of Objects
    for (i = 0; i < object_count; i++)
    {
        // Find midpoint of angles
        midpoint_angle = (final_angle[i] - initial_angle[i]) / 2
                + initial_angle[i];
        // Take reading at midpoint
        servo_move(midpoint_angle);
        timer_waitMillis(1000);
        servo_move(midpoint_angle);
        // store PING value from reading
        object_distance[i] = ping_read();
        lcd_printf("%.0f", object_distance[i]);

    }

    //LINEAR DIST
    float delta_angle;
    for (i = 0; i < object_count; i++)
    {
        delta_angle = (final_angle[i] - initial_angle[i]) / 2; //POSSIBLE FIXME
        //other_angle = (180 - delta_angle)/2;
        // linear_distance[i] = sqrt(pow(object_distance[i],2) + pow(object_distance[i],2) * object_distance[i] * object_distance[i] * cos(delta_angle));
        //linear_distance[i] = sin(delta_angle*(M_PI/180)) * (object_distance[i] / sin(other_angle*(M_PI/180)));
        linear_distance[i] = sqrt(
                pow(object_distance[i], 2) + pow(object_distance[i], 2)
                        - (2 * object_distance[i] * object_distance[i]
                                * cos(delta_angle * (M_PI / 180))));
        //linear_distance[i] = 2 * object_distance[i] * sin(delta_angle*(M_PI/180));
    }
    // send new header
    /*cybotSendString("Object #\tInitial Angle\tFinal Angle\t Distance\tLinear Width\t initial dist\tfinal dist\n\r");
     for(i = 0; i < object_count; i++){
     sprintf(scanned_data, "%d\t\t%d\t\t%d\t\t%.0f\t\t%.2f\t\t%0.0f\t\t%0.0f\n\r", (i + 1), initial_angle[i],final_angle[i], object_distance[i], linear_distance[i],initial_dist[i],final_dist[i]); // add linear distance afterwards );
     cybotSendString(scanned_data);
     }*/
    // point sensor to smallest width object
    min = linear_distance[0];
    min_inital_angle = initial_angle[0];
    min_final_angle = final_angle[0];

    for (i = 0; i < object_count; i++)
    {

        if (linear_distance[i] < min)
        {
            min = linear_distance[i];
            min_inital_angle = initial_angle[i];
            min_final_angle = final_angle[i];
        }
    }

    midpoint_angle = (min_final_angle - min_inital_angle) / 2
            + min_inital_angle;

    servo_move(midpoint_angle);

    return midpoint_angle;
}

int fullScan(int startDeg, int endDeg)
{
    //scan variable declaration
    char scanned_data[45];
    float scanned_distance_vals_PING[4];
    float scanned_distance_vals_IR[4];
    float val[5], temp;
    int j;
    int scanned_degree_vals[4], midpoint_angle;
    int alreadyRec = 0;
    float min;
    int min_inital_angle, min_final_angle;

    // Variables for object count FIXME move to correct scope (m)
    int object_count = 0;
    float object_distance[5], delta_distance; // lets try not to hard code a finite amount of these objects to make future labs easier
    int initial_angle[5]; // lets try not to hard code a finite amount of these objects to make future labs easier
    int final_angle[5];
    float initial_dist[5];
    float final_dist[5];
    float linear_distance[5];
    // cyBOT_Scan_t collected_data;

    //initialize for IR

    //char buffer[10];
    object_count = 0;
    alreadyRec = 0; // reset to 0 before scan
    oi_setWheels(0, 0);
    int i = 2;
    int curr_degree = startDeg;
    //cybotSendString("Degrees\t\tDistance IR\n");
    if (curr_degree == 0)
    {
        servo_move(0);
        timer_waitMillis(300); // FIX ME
        // Hard set point 1 of scan
        servo_move(0);
        scanned_distance_vals_IR[0] = 250; // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[0] = 0; // take account for what degree you are located at
        servo_move(2);
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = 250; // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[1] = 2; // take account for what degree you are located at
        curr_degree = 4;
    }
    else
    {
        servo_move(startDeg);
        timer_waitMillis(300);
        // Hard set point 1 of scan
        servo_move(startDeg);
        scanned_distance_vals_IR[0] = clean_IR_val(); // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[0] = startDeg; // take account for what degree you are located at
        servo_move((startDeg + 2));
        //hard set point 2 of array
        scanned_distance_vals_IR[1] = clean_IR_val(); // find the distance away from IR sensor at given degree for IR and translate it
        scanned_degree_vals[1] = startDeg + 2; // take account for what degree you are located at
        curr_degree = startDeg + 4;

    }
    // collect data
    while (curr_degree <= endDeg)
    {
        servo_move(curr_degree); // scanning
        // take in all values for current scan
        scanned_distance_vals_PING[2] = ping_read(); // find the distance away from IR sensor at given degree for PING
        // take in 3 values at current angle and translate to cm
        timer_waitMillis(10);
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

        if (scanned_distance_vals_IR[2] > 100)
        {
            scanned_distance_vals_IR[2] = 250;
        }

        // take account for what degree you are located at
        scanned_degree_vals[2] = curr_degree;

        // clean middle piece of data i.e data in scanned_distance_vals[1]
        //clean PING
        //      if ( scanned_distance_vals_PING[0] < (scanned_distance_vals_PING[2] + 20) && scanned_distance_vals_PING[0] > (scanned_distance_vals_PING[2] - 20)){
        //                  scanned_distance_vals_PING[1] = (scanned_distance_vals_PING[0] + scanned_distance_vals_PING[2]) / 2; // Matt is paranoid............
        //      }
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
            if (delta_distance <= -30 && alreadyRec == 0)
            { // falling edge, if there is a change in distance that is drastic
                alreadyRec = 1;
                initial_angle[object_count] = scanned_degree_vals[0]; // take initial angle measurement
                initial_dist[object_count] = scanned_distance_vals_IR[1]; //record that new radius away ///FIX ME CHANGED SCNDDIST_IR from 0 to 1

            }
            else if (delta_distance >= 30 && alreadyRec == 1)
            { //rising edge, if there is a change in distance that is drastic
                final_angle[object_count] = scanned_degree_vals[0]; // record the final angle of the object
                final_dist[object_count] = scanned_distance_vals_IR[1];
                if (final_angle[object_count] - initial_angle[object_count] <= 2
                        || final_angle[object_count]
                                - initial_angle[object_count] >= 75)
                { ///FIX ME CHANGED 6 to 2 in angle count

                }
                else
                {
                    object_count++; // record new object
                    alreadyRec = 0;
                }

            }
        }
        //send data to UART
        sprintf(scanned_data, "%d\t\t%.0f\t\n\r", scanned_degree_vals[0],
                scanned_distance_vals_IR[0]);
        //cybotSendString(scanned_data);

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

        curr_degree = curr_degree + 2; //Increment to collect data every 2 degrees
        timer_waitMillis(10);

    }
    // send last 2 values
    sprintf(scanned_data, "%d\t\t%.0f\n\r", scanned_degree_vals[1],
            scanned_distance_vals_IR[1]);
    //cybotSendString(scanned_data);
    sprintf(scanned_data, "%d\t\t%.0f\n\r", scanned_degree_vals[2],
            scanned_distance_vals_IR[2]);
    //cybotSendString(scanned_data);

    // PING to Center of Objects
    for (i = 0; i < object_count; i++)
    {
        // Find midpoint of angles
        midpoint_angle = (final_angle[i] - initial_angle[i]) / 2
                + initial_angle[i];
        // Take reading at midpoint
        servo_move(midpoint_angle);
        timer_waitMillis(1000);
        servo_move(midpoint_angle);
        // store PING value from reading
        object_distance[i] = ping_read();
        lcd_printf("%.0f", object_distance[i]);

    }

    //LINEAR DIST
    float delta_angle;
    for (i = 0; i < object_count; i++)
    {
        delta_angle = (final_angle[i] - initial_angle[i]) / 2; //POSSIBLE FIXME
        //other_angle = (180 - delta_angle)/2;
        // linear_distance[i] = sqrt(pow(object_distance[i],2) + pow(object_distance[i],2) * object_distance[i] * object_distance[i] * cos(delta_angle));
        //linear_distance[i] = sin(delta_angle*(M_PI/180)) * (object_distance[i] / sin(other_angle*(M_PI/180)));
        linear_distance[i] = sqrt(
                pow(object_distance[i], 2) + pow(object_distance[i], 2)
                        - (2 * object_distance[i] * object_distance[i]
                                * cos(delta_angle * (M_PI / 180))));
        //linear_distance[i] = 2 * object_distance[i] * sin(delta_angle*(M_PI/180));
    }
    // send new header
    /*cybotSendString("Object #\tInitial Angle\tFinal Angle\t Distance\tLinear Width\t initial dist\tfinal dist\n\r");
     for(i = 0; i < object_count; i++){
     sprintf(scanned_data, "%d\t\t%d\t\t%d\t\t%.0f\t\t%.2f\t\t%0.0f\t\t%0.0f\n\r", (i + 1), initial_angle[i],final_angle[i], object_distance[i], linear_distance[i],initial_dist[i],final_dist[i]); // add linear distance afterwards );
     cybotSendString(scanned_data);
     }*/
    // point sensor to smallest width object
    min = linear_distance[0];
    min_inital_angle = initial_angle[0];
    min_final_angle = final_angle[0];

    for (i = 0; i < object_count; i++)
    {

        if (linear_distance[i] < min)
        {
            min = linear_distance[i];
            min_inital_angle = initial_angle[i];
            min_final_angle = final_angle[i];
        }
    }

    midpoint_angle = (min_final_angle - min_inital_angle) / 2
            + min_inital_angle;

    servo_move(midpoint_angle);

    return midpoint_angle;
}

