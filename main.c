#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"
#include "math.h"
#include "PID.h"
#include "IMU.H"

//PID Stuff
static float angle_difference(float target_angle, float current_angle);
/* Controller parameters */

int turn_degrees(oi_t *sensor_data, float degrees);



int main(void){

        lcd_init();
        timer_init();

        oi_t *sensor_data = oi_alloc();
        oi_init(sensor_data);
        oi_setWheels(0, 0);

        I2C1_Init();
        BNO055_Init();

    turn_degrees(sensor_data, 10);
    turn_degrees(sensor_data, -10);
    turn_degrees(sensor_data, 65);
    turn_degrees(sensor_data, 100);

    oi_free
    return 0;
}


int turn_degrees(oi_t *sensor_data, float degrees) {
    // Initialize variables
    int16_t base_speed = 0;
    const int16_t desired_base_speed = 75; // Target base speed (mm/s)

    // Calculate initial heading
    float initial_heading = calculate_average_heading(10);
    float desired_heading = initial_heading + degrees;
    float current_heading = initial_heading;

    // Determine turn direction
    int turn_direction = (degrees > 0) ? 1 : -1;

    // Turn function
    while (turn_direction > 0 ? (current_heading < desired_heading) : (current_heading > desired_heading)) {
        oi_update(sensor_data);
        current_heading = calculate_average_heading(10);

        // Speed adjustment logic
        if (base_speed < desired_base_speed) {
            base_speed += 7;
            base_speed = (base_speed > desired_base_speed) ? desired_base_speed : base_speed;
        }

        // Set wheel speeds for turning
        int16_t right_speed = turn_direction * base_speed;
        int16_t left_speed = -turn_direction * base_speed;
        oi_setWheels(right_speed, left_speed);

        // Optional: Add a safety timeout
        static int timeout_counter = 0;
        timeout_counter++;
        if (timeout_counter > 100) {  // Prevent infinite loop
            oi_setWheels(0, 0);
            return 0;  // Turn failed
        }
    }

    // Stop the robot
    oi_setWheels(0, 0);
    return 1;  // Turn successful
}

//
//
//
//
//    float heading = 0;
//
//
//    while (1)
//    {
//        heading = read_euler_heading(BNO055_ADDRESS_B) * (1.0f / 16.0f);
//        timer_waitMillis(50);
//        if (heading >= -45 && heading < 45)
//        {
//            lcd_printf("North");
//        }
//        else if (heading > 45 && heading < 135)
//        {
//            lcd_printf("East");
//        }
//        else if (heading > -135 && heading < -45)
//        {
//            lcd_printf("West");
//        }
//        else if (heading >= 135 && heading < -135)
//        {
//            printf("South");
//        }
//    }
//
//
//
//
//
////////////////////////
//    /* Initialise PID controller */
//       PIDController pid = { PID_KP, PID_KI, PID_KD,
//                             PID_TAU,
//                             PID_LIM_MIN, PID_LIM_MAX};
//
//       PIDController_Init(&pid);
//
//
//    // Read and average initial heading
//     float heading_sum = 0.0f;
//     int num_readings = 10;
//         int i;
//     for (i = 0; i < num_readings; i++) {
//         heading_sum += read_euler_heading(BNO055_ADDRESS_B) * (1.0f / 16.0f);
//         timer_waitMillis(50);
//     }
//     float desired_heading = heading_sum / num_readings;
//
//     // Initialize variables
//     float current_heading = desired_heading;
//
//
//     // Display initial headings
//     printf("Initial Desired Heading: %.2f\n", desired_heading);
//     printf("Initial Current Heading: %.2f\n", current_heading);
//
//
//     float prev_error = 0.0f;
//     float integral = 0.0f;
//
//    int total_distance = 0;            // Total distance traveled in mm
//    int distance_since_last_stop = 0;  // Distance since last stop in mm
//    const int stop_interval = 600;     // 60 cm in mm
//    const int max_total_distance = 3960; // 3.96 m in mm
//
//    // Define speed variables
//    int16_t base_speed = 0;               // Starting from 0 speed
//    const int16_t desired_base_speed = 75; // Target base speed (mm/s)
//    const int16_t ramp_increment = 5;    // Speed increment per loop (mm/s)
//    const int16_t max_wheel_speed = 300; // Max wheel speed per Roomba specs (mm/s)
//
//    while (total_distance < max_total_distance) {
//
//            oi_update(sensor_data);
//
//            // Read current heading
//
//            float heading_sum = 0.0f;
//            float num_readings = 10;
//            int i;
//
//            for (i = 0; i < num_readings; i++) {
//                 heading_sum += read_euler_heading(BNO055_ADDRESS_B) * (1.0f / 16.0f);
//                 timer_waitMillis(50);
//             }
//             float current_heading = heading_sum / num_readings;
//
//
//            // Compute heading error
//            float error = angle_difference(desired_heading, current_heading);
//
//            //PIDController_Update(PIDController *pid, desired_heading, current_heading);
//
//            int16_t correct_angle_back;
//
//            // Adjust wheel speeds
//            int16_t right_speed = base_speed + (int16_t)correct_angle_back;
//            int16_t left_speed =  (base_speed - (int16_t)correct_angle_back);
//
//            // Set wheel speeds
//            oi_setWheels(right_speed, left_speed);
//
//            // Accumulate distances
//            total_distance += sensor_data->distance;
//            distance_since_last_stop += sensor_data->distance;
//            sensor_data->distance = 0;
//
//            // Display debug information
//            lcd_printf("Heading: %.2f\nError: %.2f\nDist: %d mm", current_heading, error, total_distance);
//
//            // Wait for next iteration
//            timer_waitMillis((uint32_t)(PID_Sampling * 1000));
//
//            // Handle stop conditions
//            if (distance_since_last_stop >= stop_interval) {
//                oi_setWheels(0, 0);
//                base_speed = 0;  // Reset base speed for ramp-up
//                timer_waitMillis(1000);
//                distance_since_last_stop = 0;
//            }
//
//            // Ramp up speed if not at desired speed
//            if (base_speed < desired_base_speed) {
//                base_speed += 7;  // Increment by 5 mm/s
//                if (base_speed > desired_base_speed) {
//                    base_speed = desired_base_speed;
//                }
//            }
//        }
//
//        // Stop the robot
//        oi_setWheels(0, 0);
//
//        // Free resources
//        oi_free(sensor_data);
//
//        return 0;
//    }
//    while (1)
//    {
//        x_axis = (float) read_linear_acceleration_x(0x29) * 0.01; // Read and scale linear acceleration
//        timer_waitMillis(1);
//        y_axis = (float) read_linear_acceleration_y(0x29) * 0.01;
//        timer_waitMillis(1);
//        z_axis = (float) read_linear_acceleration_y(0x29) * 0.01;
//        timer_waitMillis(1);
//        mag = sqrt(pow(x_axis, 2) + pow(y_axis, 2) + pow(z_axis, 2));
//
//        //lcd_printf("%d",count);
//        lcd_printf("- X: %.2f\n- Y: %.2f\n- Z: %.2f\n", x_axis, y_axis, z_axis);
//        timer_waitMillis(500);
//    }


//       while (1)
//       {
//           x_g_vector = (float)read_grav_vec_x(0x29) * 0.01; // Read and scale linear acceleration
//           timer_waitMillis(1);
//           y_g_vector = (float)read_grav_vec_y(0x29) * 0.01;
//           timer_waitMillis(1);
//           z_g_vector = (float)read_grav_vec_z(0x29) * 0.01;
//           timer_waitMillis(1);
//           mag = sqrt(pow(x_axis, 2) + pow(y_axis, 2) + pow(z_axis, 2));
//
//           //lcd_printf("%d",count);
//           lcd_printf("- X: %.2f\n- Y: %.2f\n- Z: %.2f\n", x_g_vector, y_g_vector, z_g_vector);
//           timer_waitMillis(500);
//       }

/////////////////////////////////////////////Functions//////////////////////////////////////////////////




/* Function to compute minimal angle difference [-180, 180] degrees */
static float angle_difference(float target_angle, float current_angle) {
    float diff = (target_angle - current_angle);
    return diff;
}

