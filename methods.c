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

#define M_PI 3.141592653589793
#define PID_KP  0.1f
#define PID_KI  0.0f
#define PID_KD  0.0f
#define PID_TAU 0.00f
#define PID_LIM_MIN -180.0f
#define PID_LIM_MAX  180.0f
#define PID_LIM_MIN_INT 0.0f
#define PID_LIM_MAX_INT  00.0f
#define SAMPLE_TIME_S 0.1f

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
float objOppDist[5];
int i, j;
// Global Variable to track which portion of the path we are currently on (1,2,3,4,5)
volatile int currPath;
volatile int hasTurned;




void moveForwardDetect(oi_t *sensor, int totalDistance) // input total Distance in MM
{
    /*
     * Move forward as long as there isn't a low object or out of bounds tape | (bumpSensor == 0) && (cliffSensor == 0)
     * Also store the distance we've traveled and subtract that from the total distance then recursively call moveForwardDetect(sensor, newDistance)
     * If IR value is greater than 1800 or less than 1200 based on testing.
     * In testing FRONT sensors were the same. Right Sensor was ~1780 on floor. Left Sensor was ~2200 on floor. All sensors gave a reading of ~2700 for the tape
     */
    float distanceTraveled = 0;
    float angle_correction = 0;
    short forward_RWP = 95; // Right wheel power for forward movement
    short forward_LWP = 80; // Left wheel power for forward movement
    short turn_RWP = 50;    // Right wheel power for turning
    short turn_LWP = 50;    // Left wheel power for turning
    int clear = sensor->distance; // clear it out
    float remainingDistance;
    char distMoved[30];
    char buffer;
    int j;
    char note[10];
    //oi_free(sensor);
    //distanceTraveled = PID_linear_movement(sensor, totalDistance);
    sensor->distance = 0;

    while ((distanceTraveled < totalDistance) && (sensor->bumpRight == 0)
            && (sensor->bumpLeft == 0)
            && ((sensor->cliffFrontLeftSignal < 2600)
                    && (sensor->cliffFrontLeftSignal > 200))
            && ((sensor->cliffFrontRightSignal < 2600)
                    && (sensor->cliffFrontRightSignal > 200))
            && ((sensor->cliffLeftSignal < 2600)
                    && (sensor->cliffLeftSignal > 200))
            && ((sensor->cliffRightSignal < 2600)
                    && (sensor->cliffRightSignal > 200)))

    {
        //oi_setWheels(100, 100); //CHANGE DURING TESTING
        //---
        Audio_Specified_Song(0x00, Im_Moving_Fwd);

        angle_correction += sensor->angle; // Accumulate angle deviation

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
        ////-------

        //---
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
        Audio_Specified_Song(0x00, Small_Obj_Hit);
        moveBackward(sensor, 100);
        turnClockwise(sensor, 86.5); //CHANGE BASED ON TESTING FIXME  //88
        moveForwardDetect(sensor, 200); //CHANGE BASED ON TESTING
        turnCounterClockwise(sensor, 86.5); //CHANGE BASED ON TESTING  //88
        fastScan(0, 180);
        if (object_count >= 1)
        {
            objectAvoid(sensor);
        }

        moveForwardDetect(sensor, (remainingDistance + 100));
    }

    else if (sensor->bumpRight == 1)
    {
        cybot_send_string("bump right\n");
        Audio_Specified_Song(0x00, Small_Obj_Hit);
        moveBackward(sensor, 100);
        turnCounterClockwise(sensor, 86.5); //CHANGE BASED ON TESTING FIXME  //88
        moveForwardDetect(sensor, 200); //CHANGE BASED ON TESTING
        turnClockwise(sensor, 86.5); //CHANGE BASED ON TESTING FIXME  //88
        fastScan(0, 180);
        if (object_count >= 1)
        {
            objectAvoid(sensor);
        }
        moveForwardDetect(sensor, (remainingDistance + 100));
    }
    if (sensor->cliffLeftSignal >= 2600 || sensor->cliffLeftSignal <= 200
            || sensor->cliffFrontLeftSignal >= 2600
            || sensor->cliffFrontLeftSignal <= 200)
    {
        if (sensor->cliffLeftSignal >= 2600
                || sensor->cliffFrontLeftSignal >= 2600)//wall
        {
            Audio_Specified_Song(0x00, wall);
            cybot_send_string("wall\n");
        }
        if (sensor->cliffLeftSignal <= 200
                || sensor->cliffFrontLeftSignal <= 200)//pit
        {
            Audio_Specified_Song(0x00, Black_hole);
            cybot_send_string("hole\n");

        }
        moveBackward(sensor, 50); //CHANGE VALUE CHANGE PLACE
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

        if (note[0] == '2' || note[1] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnCounterClockwise(sensor, 86.5); //FIXME
            turnCounterClockwise(sensor, 86.5); //FIXME
        }
        else if (note[0] == 'l' || note[1] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor, 86.5); //FIXME
        }
        else if (note[0] == 'r' || note[1] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor, 86.5); //FIXME
        }
    }

    else if (sensor->cliffRightSignal >= 2600 || sensor->cliffRightSignal <= 200
            || sensor->cliffFrontRightSignal >= 2600
            || sensor->cliffFrontRightSignal <= 200)
    {
        if (sensor->cliffRightSignal >= 2600
                || sensor->cliffFrontRightSignal >= 2600)
        {
            Audio_Specified_Song(0x00, wall);
            cybot_send_string("wall\n");
        }
        if (sensor->cliffRightSignal <= 200
                || sensor->cliffFrontRightSignal <= 200)
        {
            Audio_Specified_Song(0x00, Black_hole);
            cybot_send_string("hole\n");

        }
        moveBackward(sensor, 50); //CHANGE VALUE CHANGE PLACE
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

        if (note[0] == '2' || note[1] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnCounterClockwise(sensor, 86.5); //FIXME
            turnCounterClockwise(sensor, 86.5); //FIXME
        }
        else if (note[0] == 'l' || note[1] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor, 86.5); //FIXME
        }
        else if (note[0] == 'r' || note[1] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor, 86.5); //FIXME
        }
    }

}

int objectCollision()
{
    //int isObject = 0;

//RETURN 1 FOR TRUE 0 FOR FALSE
//this should be used for detecting the edge boundary, hole and small objects and return what object it ran into.

//cliffSensor is in oi_parsePacket(); pg 24 of open interface spec. in resources file of 288 website.
    return 0;
}
void oneObject(oi_t *sensor)
{
    //If there is one object, turn away from the object, if it is in the range of 30 - 150 deg
    //find distance from object to determine necessity
    //    final_angle[i] - initial_angle[i]

    if (midpoint_angle[0] <= 90 && midpoint_angle[0] >= 10)
    {
        objOppDist[0] = object_distance[0]
                * sin((abs(90 - final_angle[0])) * M_PI / 180);
        //See if the object is going to be in the way or not. If it is greater than 8, it is not a problem
        if (objOppDist[0] > 22)
        {
            moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING;
            //check if there was an object to avoid
        }
        //try to move forward
        else if (object_distance[0] >= 40)
        {
            moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING;

        }
        else
        {

            //turn counterclockwise, then scan
            turnCounterClockwise(sensor, 86.5);
            fastScan(0, 180);
            objectAvoid(sensor);
            moveForwardDetect(sensor, 200); 
            turnClockwise(sensor, 86.5);

        }

    }
    else if (midpoint_angle[0] <= 90 && midpoint_angle[0] >= 170)
        {
            objOppDist[0] = object_distance[0]
                    * sin(abs((initial_angle[0] - 90)) * M_PI / 180);
            //See if the object is going to be in the way or not. If it is greater than 22, it is not a problem
            if (objOppDist[0] > 22)
            {
                moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING FIXME;
                //check if there was an object to avoid
            }
            //try to move forward
            else if (object_distance[0] >= 40)
            {
                moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING FIXME;

            }
            else
            {

                //turn counterclockwise, then scan
                turnClockwise(sensor, 86.5);
                fastScan(0, 180);
                objectAvoid(sensor);
                moveForwardDetect(sensor, 200); //FIXXME during testing
                turnCounterClockwise(sensor, 86.5);

            }

        }
}
void twoObjects(oi_t *sensor)
{

    //Find the real distances from the cybot in the tangential direction
    objOppDist[0] = object_distance[0]
            * sin((abs(90 - midpoint_angle[0])) * M_PI / 180);
    objOppDist[1] = object_distance[1]
            * sin((abs(90 - midpoint_angle[1])) * M_PI / 180);
    //if one object doesnt matter then run one Object function
    lcd_printf("%.2f , %.2f\n Dist:%.2f A: %d", objOppDist[0],objOppDist[1], object_distance[0], midpoint_angle[0]);
    if (objOppDist[0] >= 22 && objOppDist[1] >= 20){
        moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING;

    }
    else if(objOppDist[1] >= 22  ){
        oneObject(sensor);
        }
    else if(objOppDist[0] >= 22  ){
        object_distance[0] = object_distance[1];
        midpoint_angle[0] = midpoint_angle[1];
        initial_angle[0] = initial_angle[1];
        final_angle[0] = final_angle[0];
        oneObject(sensor);
    }
    else{
    //Both objects less than 90 degrees
    if ((midpoint_angle[0] < 90 && midpoint_angle[0] >= 10)
            && (midpoint_angle[1] < 90 && midpoint_angle[1] >= 10))
    {
        if (objOppDist[0] > 22  && objOppDist[1] > 22 ){
            //will miss both
            moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING;
        }
        else if (objOppDist[0] <= 22  || objOppDist[1] <= 22 )
        {
            //turn counterclockwise, then scan
            turnCounterClockwise(sensor, 86.5);;
            fastScan(0, 180);
            //check if there was an object to avoid
            if (object_count >= 1)
            {
                //avoid the object
                objectAvoid(sensor);
            }
            //try to move forward
            else
            {
                moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING;
            }
            //turn back
            turnClockwise(sensor, 86.5); // might need to move into else statement right above
        }
    }
    //Both objects greater than 90
    else if ((midpoint_angle[0] <= 170 && midpoint_angle[0] >= 90)
            && (midpoint_angle[1] <= 170 && midpoint_angle[1] >= 90))
    {

        if (objOppDist[0] > 22  && objOppDist[1] > 22 ){
            //will miss both
            moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING FIXME;
        }
        else if (objOppDist[0] <= 22  || objOppDist[1] <= 22 )
        {
            //turn clockwise, then scan
            turnClockwise(sensor, 86.5);;
            fastScan(0, 180);
            //check if there was an object to avoid
            if (object_count >= 1)
            {
                //avoid the object
                objectAvoid(sensor);
            }
            //try to move forward
            else
            {
                moveForwardDetect(sensor, 300); //CHANGE BASED ON TESTING FIXME;
            }
            //turn back
            turnCounterClockwise(sensor, 86.5);; // might need to move into else statement right above
        }
    }
    //One object less than 90 and one object greater than 90
    else if (((midpoint_angle[1] <= 170 && midpoint_angle[1] >= 90)
            && (midpoint_angle[0] < 90 && midpoint_angle[0] >= 10)))
    {

        if (objOppDist[0] >= 22  && objOppDist[1] >= 22 )
        { //will miss both objects when continuing straight
            moveForwardDetect(sensor, 300);

        }
        else if (objOppDist[0] < 22  || objOppDist[1] < 22 )
        {
            if (objOppDist[0] < objOppDist[1])
            { // object 0 is more in the way for bot
                turnClockwise(sensor, 86.5);
                fastScan(0, 180);
            }
            else if (objOppDist[0] >= objOppDist[1])
            { // object 1 is more in the way for bot
                turnCounterClockwise(sensor, 86.5); //counter clockwise
                fastScan(0, 180);
            }
        }
    }
    }
}
void moreObjects(oi_t *sensor)
{
    int i = 0;
    int small = 0;
    int large = 0;
    while (i < object_count)
    {
        objOppDist[i] = object_distance[i]
                * sin((abs(90 - midpoint_angle[i])) * M_PI / 180);
        i++;
    }
    if (i == 3)
    {
        if (objOppDist[0] >= 22  && objOppDist[1] >= 22  && objOppDist[2] >= 22 )
        { //will miss all objects when continuing straight, BANG!!!!
            moveForwardDetect(sensor, 300);
        }
        else if (objOppDist[0] <= 22  || objOppDist[1] <= 22  || objOppDist[2] <= 22 )
        { //can't continue straight
            large = midpoint_angle[2] - 90;
            small = 90 - midpoint_angle[0];
            if (small > large)
            {
                turnCounterClockwise(sensor, 86.5);;
                fastScan(0, 180);
            }
            else
            {
                turnClockwise(sensor, 86.5);;
                fastScan(0, 180);
            }

        }

    }
    else if (i == 4)
    {
        if (objOppDist[0] >= 22  && objOppDist[1] >= 22  && objOppDist[2] >= 22
                && objOppDist[3] >= 22 )
        { //will miss all objects when continuing straight, BANG!!!!
            moveForwardDetect(sensor, 300);
        }
        else if (objOppDist[0] <= 22  || objOppDist[1] <= 22  || objOppDist[2] <= 22 )
        { //can't continue straight
            large = midpoint_angle[3] - 90;
            small = 90 - midpoint_angle[0];
            if (small > large)
            {
                turnCounterClockwise(sensor, 86.5);
                fastScan(0, 180);
            }
            else
            {
                turnClockwise(sensor, 86.5);;
                fastScan(0, 180);
            }

        }

    }
}

void objectAvoid(oi_t *sensor)
{

    lcd_printf("%d", object_count);
    //This Function turns the robot to avoid obstacles It uses CurrPath to decide which way it should be turning
    //Should turn the way there are less objects
    if (object_count == 1)
    {
        oneObject(sensor);
    }
    else if (object_count == 2)
    {
        twoObjects(sensor);
    }
    else // 3 or 4 objects in front!?!?!?
    {
        moreObjects(sensor);
    }
}



int fastScan(int startDeg, int endDeg)
{
//scan variable declaration
    cybot_send_string("scanning\n");
    char scanned_data[45];
    float scanned_distance_vals_PING[4];
    float scanned_distance_vals_IR[4];
    float val[5], temp;
    int j;
    int scanned_degree_vals[4];
    int alreadyRec = 0;
    int pingDist;
    int heading;
// Variables for object count FIXME move to correct scope (m)
    //heading = (int)((float)read_euler_heading(0x29) / 16.0);

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
        // Take PING to check distance
        pingDist = ping_read();

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
        //cybot_send_string(scanned_data);

        //Receive Uart Commands

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
        sprintf(scanned_data, "%.2f\t%d\t%d\n", object_distance[i],
                midpoint_angle[i], heading);
        cybot_send_string(scanned_data);
    }
// tell GUI there are no more objects
    cybot_send_string("finished\n");

    return 1;
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
        if (note[0] == 'f')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 86.5); //FIXME
            turnClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnClockwise(sensor_data, 86.5);
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnCounterClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'q')
        {
            cybot_send_string("stop\n");
            break;
        }
        else if (note[0] == 'j')
        {
            Audio_Specified_Song(0x00, Joke_1);
        }
        //check if there was an object to avoid
        fastScan(0, 180);

        if (object_count >= 1)
        {
            //avoid the object
            objectAvoid(sensor_data);
        }
        fastScan(0, 180);
        moveForwardDetect(sensor_data, 500);
    }
  return;
}

void find_kitchen(oi_t *sensor_data)
{
    currPath = 2;
    hasTurned = 0;
    char buffer;
    int j;
    char note[10];
//    while (1)
//    {
//        cybot_send_string("beginning\n");
//        buffer = uart_receive();
//        j = 0;
//        while (buffer != '\n')
//        {
//            note[j] = buffer;
//            lcd_putc(note[j]);
//            j++;
//            buffer = uart_receive();
//        }
//        lcd_printf("Note: %s", note);
//    }
    //find kitchen -- Needs to make 180 degree turn and go back to start then turn right 90 degrees and go straight down
    //Turn 180 Degrees
    turnCounterClockwise(sensor_data, 86.5); //FIXME
    turnCounterClockwise(sensor_data, 86.5); //FIXME
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
        if (note[0] == 'f')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 86.5); //FIXME
            turnClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 86.5); //FIXME
        }
    }
//Now Turn and get up to Kitchen
    turnClockwise(sensor_data, 86.5); //CHANGE BASED ON TESTING  //88
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
        if (note[0] == 'f')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 86.5); //FIXME
            turnClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 86.5); //FIXME
        }
    }
return;
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
    turnClockwise(sensor_data, 86.5); //CHANGE BASED ON TESTING  //88
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
        if (note[0] == 'f')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 86.5); //FIXME
            turnClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 86.5); //FIXME
        }
    }
    return;
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
    turnClockwise(sensor_data, 86.5); //CHANGE BASED ON TESTING  //88
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
        if (note[0] == 'f')
        {
            //at destination
            break;
        }
        else if (note[0] == '2')
        {
            //facing wrong direction turn 2 times CW
            turnClockwise(sensor_data, 86.5); //FIXME
            turnClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'l')
        {
            //turn left or 1 CCW
            turnCounterClockwise(sensor_data, 86.5); //FIXME
        }
        else if (note[0] == 'r')
        {
            //turn right or 1 CW
            turnClockwise(sensor_data, 86.5); //FIXME
        }
    }
    return;
}

void autoManualDriver(oi_t *sensor_data)
{
    char message[50];
    char buffer;
    int j;
    lcd_clear();
    oi_setWheels(0, 0);
    while (1)
    {
        if(flag1 == 1){
            break;
        }
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
            Audio_Specified_Song(0x00, Good_Morning);
            find_bathroom(sensor_data);
            Audio_Specified_Song(0x00, We_have_reached_the_Bathroom);
            break;
        case '2':
            //find kitchen -- Needs to make 180 degree turn and go back to start then turn right 90 degrees and go straight down
            find_kitchen(sensor_data);
            Audio_Specified_Song(0x00, We_have_reached_the_Kitchen);
            break;
        case '3':
            //find Living Room --
            find_livingRoom(sensor_data);
            Audio_Specified_Song(0x00, We_have_reached_the_Living_Room);
            break;
        case '4':
            //find Exit --
            find_exit(sensor_data);
            Audio_Specified_Song(0x00, We_have_reached_the_front_door);
            timer_waitMillis(6000);
            Audio_Specified_Song(0x00, end);
            break;
        case 'x':
            Audio_Specified_Song(0x00, Are_You_Ready);
            cybot_send_string("Playing audio\n");
            break;
        case 'w':
            moveForward(sensor_data, 500);
            lcd_clear();
            cybot_send_string("Driving Forward\n");
            break;

        case 'a':
            turnCounterClockwise(sensor_data, 86.5);
            lcd_clear();
            cybot_send_string("Turning Left\n");
            break;
        case 'd':
            turnClockwise(sensor_data, 86.5);
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
            Audio_Specified_Song(0x00, Stopping_Now);
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
    return;
}

//void PIDController_Init(PIDController *pid)
//{
//
//    /* Clear controller variables */
//    pid->integrator = 0.0f;
//    pid->prevError = 0.0f;
//    pid->differentiator = 0.0f;
//    pid->prevMeasurement = 0.0f;
//    pid->out = 0.0f;
//
//}
//
//float PIDController_Update(PIDController *pid, float setpoint,
//                           float measurement)
//{
//
//    /*
//     * Error signal (handling angle wrapping)
//     */
//    float error = angle_difference(setpoint, measurement);
//
//    /*
//     * Proportional
//     */
//    float proportional = pid->Kp * error;
//
//    /*
//     * Integral
//     */
//    pid->integrator += 0.5f * pid->Ki * pid->T * (error + pid->prevError);
//
//    /* Anti-wind-up via integrator clamping */
//    if (pid->integrator > pid->limMaxInt)
//    {
//
//        pid->integrator = pid->limMaxInt;
//
//    }
//    else if (pid->integrator < pid->limMinInt)
//    {
//
//        pid->integrator = pid->limMinInt;
//    }
//
//    /*
//     * Derivative (band-limited differentiator)
//     */
//    float delta_measurement = angle_difference(measurement,
//                                               pid->prevMeasurement);
//
//    pid->differentiator = -(2.0f * pid->Kd * delta_measurement /* Note: derivative on measurement! */
//    + (2.0f * pid->tau - pid->T) * pid->differentiator)
//            / (2.0f * pid->tau + pid->T);
//
//    /*
//     * Compute output and apply limits
//     */
//    pid->out = proportional; //+ pid->integrator + pid->differentiator;
//
//    if (pid->out > pid->limMax)
//    {
//        pid->out = pid->limMax;
//    }
//    else if (pid->out < pid->limMin)
//    {
//        pid->out = pid->limMin;
//    }
//
//    /* Store error and measurement for later use */
//    pid->prevError = error;
//    pid->prevMeasurement = measurement;
//
//    /* Return controller output */
//    return pid->out;
//
//}
//
//void bubble_sort(float arr[], int n)
//{
//    for (i = 0; i < n - 1; i++)
//    {
//        for (j = 0; j < n - i - 1; j++)
//        {
//            if (arr[j] > arr[j + 1])
//            {
//                // Swap arr[j] and arr[j + 1]
//                float temp = arr[j];
//                arr[j] = arr[j + 1];
//                arr[j + 1] = temp;
//            }
//        }
//    }
//}
//
//float PID_linear_movement(oi_t *sensor, int totalDistance)
//{
//
//    float distanceTraveled = 0;
//    int16_t correct_angle_back, right_speed, left_speed;
//    // resets the distance for the oi
//    //int reset = sensor->distance;
//    int i;
//    float current_heading, error, heading_sum, desired_heading;
//    oi_setWheels(0, 0);
//
//    /* Initialize PID controller */
//    PIDController pid = { PID_KP, PID_KI, PID_KD, PID_TAU, PID_LIM_MIN,
//                          PID_LIM_MAX, PID_LIM_MIN_INT, PID_LIM_MAX_INT,
//                          SAMPLE_TIME_S };
//
//    PIDController_Init(&pid);
//
//    // Read and average initial heading
//    float heading_readings[5];
//    heading_sum = 0.0f;
//    int num_readings = 5;
//    for (i = 0; i < num_readings; i++)
//    {
//        heading_readings[i] = read_euler_heading(BNO055_ADDRESS_B)
//                * (1.0f / 16.0f);
//        timer_waitMillis(5);
//    }
//
//    // Sort the array using bubble sort
//    bubble_sort(heading_readings, num_readings);
//
//    // Calculate the average of the middle three readings
//    for (i = 1; i < num_readings - 1; i++)
//    { // Skip the lowest and highest
//        heading_sum += heading_readings[i];
//    }
//    desired_heading = heading_sum / 3.0f;
//
//    // Initialize variables
//    current_heading = desired_heading;
//
//    // Display initial headings
//    printf("Initial Desired Heading: %.2f\n", desired_heading);
//    printf("Initial Current Heading: %.2f\n", current_heading);
//
//    //float prev_error = 0.0f;
//    //float integral = 0.0f;
//    // Define speed variables
//    int16_t base_speed = 0;               // Starting from 0 speed
//    const int16_t desired_base_speed = 75; // Target base speed (mm/s)
//    //const int16_t ramp_increment = 5;    // Speed increment per loop (mm/s)
//    //const int16_t max_wheel_speed = 300; // Max wheel speed per Roomba specs (mm/s)
//
//    sensor->distance = 0;
//
//    while ((distanceTraveled < totalDistance) //&& (sensor->bumpRight == 0)
////            && (sensor->bumpLeft == 0)
////            && ((sensor->cliffFrontLeftSignal < 2500)
////                    && (sensor->cliffFrontLeftSignal > 200))
////            && ((sensor->cliffFrontRightSignal < 2500)
////                    && (sensor->cliffFrontRightSignal > 200))
////            && ((sensor->cliffLeftSignal < 2500)
////                    && (sensor->cliffLeftSignal > 200))
////            && ((sensor->cliffRightSignal < 2500)
////                    && (sensor->cliffRightSignal > 200))
//    )
//
//    {
//        oi_update(sensor);
//
//        // Read current heading
//
//        heading_sum = 0.0f;
//        num_readings = 10;
//
//        for (i = 0; i < num_readings; i++)
//        {
//            oi_update(sensor);
//            heading_sum += read_euler_heading(BNO055_ADDRESS_B)
//                    * (1.0f / 16.0f);
//            timer_waitMillis(50);
//        }
//        current_heading = heading_sum / (float) num_readings;
//
//        // Compute heading error
//        error = angle_difference(desired_heading, current_heading);
//        if ((totalDistance - distanceTraveled) < 5)
//        {
//            //slow down before end
//            base_speed = desired_base_speed
//                    * ((totalDistance - distanceTraveled) / 5.0f);
//        }
//        if (error >= 8)
//        {
//            correct_angle_back = PIDController_Update(&pid, desired_heading,
//                                                      current_heading); //CHANGED TO = 0; INSTEAD OF ;
//        }
//        else
//        {
//            correct_angle_back = 0;
//        }
//        // Adjust wheel speeds
//        right_speed = base_speed - (int16_t) correct_angle_back; //
//        left_speed = (base_speed + (int16_t) correct_angle_back); //
//
//        // Set wheel speeds
//        oi_setWheels(right_speed, left_speed);
//        // Accumulate distances
//        distanceTraveled += sensor->distance;
//
//        // Display debug information
//        lcd_printf("Error: %.2f\ndist travl: %.2f\n Td: %d\nCAB: %d", error,
//                   distanceTraveled, totalDistance,
//                   (int16_t) correct_angle_back);
//        //Dist: %d mm\n
//        // Wait for next iteration
//        timer_waitMillis((uint32_t) (SAMPLE_TIME_S * 1000));
//        oi_update(sensor);
//
//        base_speed = desired_base_speed;
//
//    }
//    lcd_clear();
//    oi_setWheels(0, 0);
//    lcd_printf("BL: %d\nBR: %d\nCFL: %d\nCFR: %d\n", sensor->bumpLeft,
//               sensor->bumpRight, sensor->cliffFrontLeftSignal,
//               sensor->cliffFrontRightSignal);
//    timer_waitMillis(5000);
//    return distanceTraveled;
//
//}
//
//float angle_difference(float target_angle, float current_angle)
//{
//    float diff = (target_angle - current_angle);
//    while (diff > 180.0f)
//        diff -= 360.0f;
//    while (diff < -180.0f)
//        diff += 360.0f;
//    return diff;
//}
//
//float calculate_average_heading(void)
//{
//
//    float heading_sum = 0.0f;
//    float num_readings = 5;
//    int i;
//
//    for (i = 0; i < num_readings; i++)
//    {
//        heading_sum += read_euler_heading(BNO055_ADDRESS_B) * (1.0f / 16.0f);
//        timer_waitMillis(50);
//    }
//    float current_heading = heading_sum / num_readings;
//    return current_heading;
//}
//
//int turn_degrees(oi_t *sensor_data, float degrees)
//{
//    // Clear LCD and provide direction feedback
//    lcd_clear();
//    if (degrees == 90)
//    {
//        cybot_send_string("clockwise\n");
//    }
//    else if (degrees == -90)
//    {
//        cybot_send_string("counterclockwise\n");
//    }
//
//    // Calculate initial and desired headings
//    float initial_heading = calculate_average_heading();
//    float desired_heading = initial_heading + degrees;
//
//    // Normalize desired heading to 0-360 range
//    while (desired_heading > 360.0)
//    {
//        desired_heading -= 360.0;
//    }
//    while (desired_heading < 0.0)
//    {
//        desired_heading += 360.0;
//    }
//
//    // Determine turn direction
//    int turn_direction = (degrees > 0) ? 1 : -1;
//
//    // Fixed turning speed
//    const int16_t TURN_SPEED = 50;
//
//    // Heading tolerance and timeout parameters
//    const float HEADING_TOLERANCE = 5.0;  // 2-degree tolerance
//    const int MAX_TIMEOUT = 100;
//
//    float current_heading = initial_heading;
//    int timeout_counter = 0;
//
//    // Turn function
//    while (true)
//    {
//        // Update sensor data and current heading
//        oi_update(sensor_data);
//        current_heading = calculate_average_heading();
//        lcd_printf("desired heading %.2f\nCurrent heading: %.2f\n",
//                   desired_heading, current_heading);
//        // Calculate heading error
//        float heading_error = abs(desired_heading - current_heading);
//
//        // Check if turn is complete within tolerance
//        if (heading_error <= HEADING_TOLERANCE)
//        {
//            break;
//        }
//
//        // Set wheel speeds for turning
//        int16_t right_speed = -1 * turn_direction * TURN_SPEED;
//        int16_t left_speed = turn_direction * TURN_SPEED;
//        oi_setWheels(right_speed, left_speed);
//
//        // Timeout prevention
//        timeout_counter++;
//        if (timeout_counter > MAX_TIMEOUT)
//        {
//            lcd_printf("Turn Timeout: Failed\n");
//            oi_setWheels(0, 0);
//            return 0;  // Turn failed
//        }
//    }
//
//    // Stop the robot
//    oi_setWheels(0, 0);
//
//    // Verify final heading
//    current_heading = calculate_average_heading();
//    float final_error = abs(desired_heading - current_heading);
//
//    if (final_error > HEADING_TOLERANCE)
//    {
//        lcd_printf("Turn Incomplete: Error %.2f\n", final_error);
//        return 0;
//    }
//
//    return 1;  // Turn successful
//}
