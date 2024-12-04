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
//Global variables for objects
int object_count;
float object_distance[5];
int initial_angle[5];
int midpoint_angle[5];
int final_angle[5];
// Global Variable to track which portion of the path we are currently on (1,2,3,4,5)
volatile int currPath;
volatile int hasTurned;

void sendToPutty(char string[], int length)
{

    int i = 0;

    for (i = 0; i < length; i++)
    {
        //cyBot_sendByte(string[i]);
    }
}

void printIR(oi_t *sensor)
{
    char array[10];
    oi_update(sensor);
    sprintf(array, "%d\n", sensor->cliffLeftSignal); // Format float to string
    sendToPutty(array, strlen(array));
}

void moveForwardDetect(oi_t *sensor, int totalDistance) // input total Distance in MM
{
    /*
     * Move forward as long as there isn't a low object or out of bounds tape | (bumpSensor == 0) && (cliffSensor == 0)
     * Also store the distance we've traveled and subtract that from the total distance then recursively call moveForwardDetect(sensor, newDistance)
     * If IR value is greater than 1800 or less than 1200 based on testing.
     * In testing FRONT sensors were the same. Right Sensor was ~1780 on floor. Left Sensor was ~2200 on floor. All sensors gave a reading of ~2700 for the tape
     */

    float distanceTraveled = 0;
    float remainingDistance = 0;
    // resets the distance for the oi
    int reset = sensor->distance;
    char distMoved[50];
    char buffer;
    int j;
    char note[10];

    while ((distanceTraveled < totalDistance) && (sensor->bumpRight == 0)
            && (sensor->bumpLeft == 0)
            && ((sensor->cliffFrontLeftSignal < 2500)
                    && (sensor->cliffFrontLeftSignal > 200))
            && ((sensor->cliffFrontRightSignal < 2500)
                    && (sensor->cliffFrontRightSignal > 200))
            && ((sensor->cliffLeftSignal < 2500)
                    && (sensor->cliffLeftSignal > 200))
            && ((sensor->cliffRightSignal < 2500)
                    && (sensor->cliffRightSignal > 200)))

    {
        oi_setWheels(105, 100); //CHANGE DURING TESTING
        oi_update(sensor);
        distanceTraveled += sensor->distance; //Sensor Distance Returns a Value in MM
        lcd_printf("Dist: %f", distanceTraveled);
    }
    //record the distance moved and send it to the GUI
    sprintf(distMoved, "moved\t%0.2f\n", distanceTraveled);
    cybot_send_string(distMoved);
    oi_setWheels(0, 0);
    remainingDistance = totalDistance - distanceTraveled;

    if (sensor->bumpLeft == 1)
    {
        cybot_send_string("bump left\n");
        moveBackward(sensor, 100);
        turnClockwise(sensor, 88.5); //CHANGE BASED ON TESTING FIXME  //88
        moveForwardDetect(sensor, 200); //CHANGE BASED ON TESTING
        turnCounterClockwise(sensor, 87.75); //CHANGE BASED ON TESTING  //88
        moveForwardDetect(sensor, (remainingDistance + 100));
    }

    else if (sensor->bumpRight == 1)
    {
        cybot_send_string("bump right\n");
        moveBackward(sensor, 100);
        turnCounterClockwise(sensor, 87.75); //88
        moveForwardDetect(sensor, 200); //CHANGE BASED ON TESTING FIXME
        turnClockwise(sensor, 88.5); //CHANGE BASED ON TESTING  //88
        moveForwardDetect(sensor, (remainingDistance + 100));
    }
    if (sensor->cliffLeftSignal >= 2500 || sensor->cliffLeftSignal <= 200
            || sensor->cliffFrontLeftSignal >= 2500
            || sensor->cliffFrontLeftSignal <= 200)
    {
        moveBackward(sensor, 50); //CHANGE VALUE
        cybot_send_string("Cliff Left Signal\n");
        buffer = uart_receive();
        j = 0;
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        lcd_printf("Note: %s", note);

        if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor, 88.5); //FIXME
            turnClockwise(sensor, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor, 87.75); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor, 88.5); //FIXME
        }
    }

    else if (sensor->cliffRightSignal >= 2500 || sensor->cliffRightSignal <= 200
            || sensor->cliffFrontRightSignal >= 2500
            || sensor->cliffFrontRightSignal <= 200)
    {
        cybot_send_string("Cliff Right Signal\n");
        moveBackward(sensor, 50);
        char buffer;
        int j;
        char note[10];
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        lcd_printf("Note: %s", note);

        if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor, 88.5); //FIXME
            turnClockwise(sensor, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor, 87.75); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor, 88.5); //FIXME
        }
    }

}

int objectCollision()
{
    int isObject = 0;
//RETURN 1 FOR TRUE 0 FOR FALSE
//this should be used for detecting the edge boundary, hole and small objects and return what object it ran into.

//cliffSensor is in oi_parsePacket(); pg 24 of open interface spec. in resources file of 288 website.
    return 0;
}

void objectAvoid(oi_t *sensor)
{
//This Function turns the robot to avoid obstacles It uses CurrPath to decide which way it should be turning
//Should turn the way there are less objects
    if (object_count == 1)
    {
        //If there is one object, turn away from the object, if it is in the range of 30 - 150 deg
        if (midpoint_angle[0] <= 90 && midpoint_angle[0] >= 30
                && object_distance[0] < 55) //CHANGE DURING TESTING
        {
            //turn counterclockwise, then scan
            turnCounterClockwise(sensor, 87.75);
            fastScan(0, 180);
            //check if there was an object to avoid
            if (object_count >= 1)
            {
                //avoid the object
                secondObject(sensor);
            }
            //try to move forward
            else
            {
                moveForward(sensor, 300); //CHANGE BASED ON TESTING FIXME;
            }
            //turn back
            turnClockwise(sensor, 88.5);
        }
        else if (midpoint_angle[0] <= 150 && midpoint_angle[0] >= 90
                && object_distance[0] < 55) //CHANGE DURING TESTING
        {
            //turn clockwise, then scan
            turnClockwise(sensor, 88.5);
            fastScan(0, 180);
            //check if there was an object to avoid
            if (object_count >= 1)
            {
                //avoid the object
                secondObject(sensor);
            }
            //try to move forward
            else
            {
                moveForward(sensor, 300); //CHANGE BASED ON TESTING FIXME;
            }
            //turn back
            turnCounterClockwise(sensor, 87.75);
        }

    }
    else if (object_count == 2)
    {
        //If there is one object, turn away from the object
        //they could both be less than 90
        if ((midpoint_angle[0] < 90 && midpoint_angle[0] >= 30)
                && (midpoint_angle[1] < 90 && midpoint_angle[1] >= 30))
        {
            //turn counterclockwise, then scan
            turnCounterClockwise(sensor, 87.75);
            fastScan(0, 180);
            //check if there was an object to avoid
            if (object_count >= 1)
            {
                //avoid the object
                secondObject(sensor);
            }
            //try to move forward
            else
            {
                moveForward(sensor, 300); //CHANGE BASED ON TESTING FIXME;
            }
            //turn back
            turnClockwise(sensor, 88.5);
        }
        //they could both be greater than 90
        else if ((midpoint_angle[0] <= 150 && midpoint_angle[0] >= 90)
                && (midpoint_angle[1] <= 150 && midpoint_angle[1] >= 90))
        {
            //turn clockwise, then scan
            turnClockwise(sensor, 88.5);
            fastScan(0, 180);
            //check if there was an object to avoid
            if (object_count >= 1)
            {
                //avoid the object
                secondObject(sensor);
            }
            //try to move forward
            else
            {
                moveForward(sensor, 300); //CHANGE BASED ON TESTING FIXME;
            }
            //turn back
            turnCounterClockwise(sensor, 87.75);
        }
        //If they ain't both on one side pick the one that is closer to turn away from? Deal with the second later
        else if (object_distance[0] < object_distance[1])
        {
            //turn away from object[0]
            object_count = 1;
            objectAvoid(sensor);

        }
        else if (object_distance[0] >= object_distance[1])
        {
            //turn away from object[1]
            object_count = 1;
            midpoint_angle[0] = midpoint_angle[1];
            objectAvoid(sensor);

        }
    }
    else
    {
        int i;
        int max = object_distance[0];
        for (i = 0; i < 3; i++)
        {
            if (object_distance[i] < max)
            {
                max = object_distance[i];
            }
        }
        //I only want to deal with 3 objects
        if (midpoint_angle[0] < 20 || midpoint_angle[0] > 170)
        {
            midpoint_angle[0] = midpoint_angle[1];
            object_distance[0] = object_distance[1];
            midpoint_angle[1] = midpoint_angle[2];
            object_distance[1] = object_distance[2];
            object_count = 2;
            objectAvoid(sensor);
        }
        else if (midpoint_angle[1] < 20 || midpoint_angle[1] > 170)
        {
            object_distance[1] = object_distance[2];
            midpoint_angle[1] = midpoint_angle[2];
            object_count = 2;
            objectAvoid(sensor);
        }
        else if (midpoint_angle[2] < 20 || midpoint_angle[2] > 170)
        {
            object_count = 2;
            objectAvoid(sensor);
        }
        else if (midpoint_angle[0] == max)
        {
            midpoint_angle[0] = midpoint_angle[1];
            object_distance[0] = object_distance[1];
            midpoint_angle[1] = midpoint_angle[2];
            object_distance[1] = object_distance[2];
            object_count = 2;
            objectAvoid(sensor);
        }
        else if (midpoint_angle[1] == max)
        {
            midpoint_angle[1] = midpoint_angle[2];
            object_distance[1] = object_distance[2];
            object_count = 2;
            objectAvoid(sensor);
        }
        else if (midpoint_angle[2] == max)
        {
            object_count = 2;
            objectAvoid(sensor);
        }

    }

}

void secondObject(oi_t *sensor)
{
//The cybot is trying to avoid one object and has now seen another
    if (object_count == 1)
    {
        if (midpoint_angle[0] <= 170 && midpoint_angle[0] >= 20)
        {
            //it is right in front of us
            if (object_distance[0] > 55)
            {
                //its far enough
                moveForward(sensor, 300);
            }
            else if (object_distance[0] >= 30)
            {
                //try to move forward
                moveForward(sensor, object_distance[0] - 25);
            }
            else
            {
                //danger zone -- the object is RIGHT THERE, randomly turn until there isn't an object?
                turnClockwise(sensor, 88.5);
                fastScan(0, 180);
                //check if there was an object to avoid
                if (object_count >= 1)
                {
                    //avoid the object
                    secondObject(sensor);
                }

            }
        }
        else
        {
            //should be out of range, return to original function

        }
    }

}

int fastScan(int startDeg, int endDeg)
{
//will scan at increment of 4 degrees. we can change this to be larger as we see fit.
//scan variable declaration
    cybot_send_string("scanning\n");
    char scanned_data[45];
    float scanned_distance_vals_PING[4];
    float scanned_distance_vals_IR[4];
    float val[5], temp;
    int j;
    int scanned_degree_vals[4];
    int alreadyRec = 0;

// Variables for object count FIXME move to correct scope (m)

    float delta_distance;
    float initial_dist[5];
    float final_dist[5];
    float linear_distance[5];

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
        //cybot_send_string(scanned_data);

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
//cybot_send_string(scanned_data);
    sprintf(scanned_data, "%d\t\t%.0f\n\r", scanned_degree_vals[2],
            scanned_distance_vals_IR[2]);
//cybot_send_string(scanned_data);

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
//Clear out the array
    scanned_data[0] = '\0';
//Send Objects and distances to GUI
    for (i = 0; i < object_count; i++)
    {
        //Format Data to be sent
        //Distance from bot \t angle in perspective to bot
        sprintf(scanned_data, "%.2f\t%d\n", object_distance[i],
                midpoint_angle[i]);
        cybot_send_string(scanned_data);
    }
// tell GUI there are no more objects
    cybot_send_string("finished\n");

    return 1;
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

void find_bathroom(oi_t *sensor_data)
{
    currPath = 1;
    hasTurned = 0;
// find bathroom should move in a straight line across the bottom of the course
    char buffer;
    int j;
    char note[10];
    cybot_send_string("beginning\n");
    fastScan(0, 180);
//check if there was an object to avoid
    if (object_count >= 1)
    {
        //avoid the object
        objectAvoid(sensor_data);
    }
    while (1)
    {
        moveForwardDetect(sensor_data, 500);
        timer_waitMillis(10);
        //scan
        fastScan(0, 180);
        //check if there was an object to avoid
        if (object_count >= 1)
        {
            //avoid the object
            objectAvoid(sensor_data);
        }
        buffer = uart_receive();
        j = 0;
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        lcd_printf("Note: %s", note);
        if (note[0] == '1')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 88.5); //FIXME
            turnClockwise(sensor_data, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 87.75); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 88.5); //FIXME
        }
    }
}

void find_kitchen(oi_t *sensor_data)
{
    currPath = 2;
    hasTurned = 0;
//find kitchen -- Needs to make 180 degree turn and go back to start then turn right 90 degrees and go straight down
    char buffer;
    int j;
    char note[10];
//Turn 180 Degrees
    turnCounterClockwise(sensor_data, 87.75); //FIXME
    turnCounterClockwise(sensor_data, 87.75); //FIXME
    fastScan(0, 180);
//check if there was an object to avoid
    if (object_count >= 1)
    {
        //avoid the object
        objectAvoid(sensor_data);
    }
// first move to get back to the bedroom
    while (1)
    {
        cybot_send_string("beginning\n");
        moveForwardDetect(sensor_data, 500);
        timer_waitMillis(10);
        fastScan(0, 180);
        //check if there was an object to avoid
        if (object_count >= 1)
        {
            //avoid the object
            objectAvoid(sensor_data);
        }
        buffer = uart_receive();
        j = 0;
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        lcd_printf("Note: %s", note);
        if (note[0] == '1')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 88.5); //FIXME
            turnClockwise(sensor_data, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 87.75); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 88.5); //FIXME
        }
    }
//Now Turn and get up to Kitchen
    turnClockwise(sensor_data, 88.5); //CHANGE BASED ON TESTING  //88
    currPath = 3;
    hasTurned = 0;
    fastScan(0, 180);
//check if there was an object to avoid
    if (object_count >= 1)
    {
        //avoid the object
        objectAvoid(sensor_data);
    }
    while (1)
    {
        cybot_send_string("beginning\n");
        moveForwardDetect(sensor_data, 500);
        timer_waitMillis(10);
        fastScan(0, 180);
        //check if there was an object to avoid
        if (object_count >= 1)
        {
            //avoid the object
            objectAvoid(sensor_data);
        }
        buffer = uart_receive();
        j = 0;
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        lcd_printf("Note: %s", note);
        if (note[0] == '1')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 88.5); //FIXME
            turnClockwise(sensor_data, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 87.75); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 88.5); //FIXME
        }
    }

}

void find_livingRoom(oi_t *sensor_data)
{
    currPath = 4;
    hasTurned = 0;
// find Living Room should turn then move in a straight line across the bottom of the course
    char buffer;
    int j;
    char note[10];
//turn
    turnClockwise(sensor_data, 88.5); //CHANGE BASED ON TESTING  //88
    fastScan(0, 180);
//check if there was an object to avoid
    if (object_count >= 1)
    {
        //avoid the object
        objectAvoid(sensor_data);
    }
    while (1)
    {
        cybot_send_string("beginning\n");
        moveForwardDetect(sensor_data, 500);
        timer_waitMillis(10);
        fastScan(0, 180);
        //check if there was an object to avoid
        if (object_count >= 1)
        {
            //avoid the object
            objectAvoid(sensor_data);
        }
        cybot_send_string("moved forward\n");
        buffer = uart_receive();
        j = 0;
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        lcd_printf("Note: %s", note);
        if (note[0] == '1')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 88.5); //FIXME
            turnClockwise(sensor_data, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 87.75); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 88.5); //FIXME
        }
    }
}

void find_exit(oi_t *sensor_data)
{
    currPath = 5;
    hasTurned = 0;
// find Living Room should turn then move in a straight line across the bottom of the course
    char buffer;
    int j;
    char note[10];
//turn
    turnClockwise(sensor_data, 88.5); //CHANGE BASED ON TESTING  //88
    fastScan(0, 180);
//check if there was an object to avoid
    if (object_count >= 1)
    {
        //avoid the object
        objectAvoid(sensor_data);
    }
    while (1)
    {
        moveForwardDetect(sensor_data, 500);
        timer_waitMillis(10);
        fastScan(0, 180);
        //check if there was an object to avoid
        if (object_count >= 1)
        {
            //avoid the object
            objectAvoid(sensor_data);
        }
        cybot_send_string("moved forward\n");
        buffer = uart_receive();
        j = 0;
        while (buffer != '\n')
        {
            note[j] = buffer;
            lcd_putc(note[j]);
            j++;
            buffer = uart_receive();
        }
        lcd_printf("Note: %s", note);
        if (note[0] == '1')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 88.5); //FIXME
            turnClockwise(sensor_data, 88.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 87.75); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 88.5); //FIXME
        }
    }
}

void manualDriver(oi_t *sensor_data)
{
    char message[50];
    char buffer;
    int j;
    lcd_clear();
    oi_setWheels(0, 0);
    while (1)
    {
        j = 0;
        buffer = uart_receive();
        while (buffer != '\n')
        {
            message[j] = buffer;
            lcd_putc(message[j]);
            j++;
            buffer = uart_receive();
        }
        switch (message[0])
        {
        // switch only over the first index of the message
        case '1':
            //find bathroom
            find_bathroom(sensor_data);
            break;
        case '2':
            //find kitchen -- Needs to make 180 degree turn and go back to start then turn right 90 degrees and go straight down
            find_kitchen(sensor_data);
            break;
        case '3':
            //find Living Room --
            find_livingRoom(sensor_data);
            break;
        case '4':
            //find Exit --
            find_livingRoom(sensor_data);
            break;
        case 'x':
            //audio_play();
            cybot_send_string("Playing audio\n");
            break;
        case 'w':
            moveForward(sensor_data, 500);
            lcd_clear();
            cybot_send_string("Driving Forward\n");
            break;

        case 'a':
            turnCounterClockwise(sensor_data, 87.75);
            lcd_clear();
            cybot_send_string("Turning Left\n");
            break;
        case 'd':
            turnClockwise(sensor_data, 88.5);
            lcd_clear();
            cybot_send_string("Turning Right\n");
            break;
        case 'm':
            cybot_send_string("m\n");
            fastScan(0, 180);
            break;
        case 's':
            moveBackward(sensor_data, 500);
            lcd_clear();
            cybot_send_string("Driving Backwards\n");
            break;
        case 'q':
            oi_setWheels(0, 0);
            cybot_send_string("Stopping\n");
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

