/*
This file is the main file for quest 2, driving data collection and serial communication
Justin Nascimento (U42983905) and Alvin Yan ()
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "i2c_functions.h"
#include "ADXL343.h"
#include "garmin_lidar.h"
#include "timer_interrupt.h"
#include "ultrasonic.h"

// Mode to choose which way to print out data
enum mode{
    doubleIntegration,
    sensorFusion,
    phoneData
};

// Global Mode Variable
enum mode mode = doubleIntegration;

// Double Integration Task to perform double integration to get distance data
static void double_integration_task(void *arg){
    int evt;
    int counter = 0;
    struct accelerationVals tempAcceleration;
    struct accelerationVals averageAcceleration;

    averageAcceleration.accelX = 0;
    averageAcceleration.accelY = 0;
    averageAcceleration.accelZ = 0;

    while(1){
        // On the timer, collect data
        if (xQueueReceive(timer_queue, &evt, portMAX_DELAY)) {

            // Collect Acceleration Data
            tempAcceleration = getAccel();

            averageAcceleration.accelX += tempAcceleration.accelX;
            averageAcceleration.accelY += tempAcceleration.accelY;
            averageAcceleration.accelZ += tempAcceleration.accelZ;

            // Increment counter
            counter++;

            // Every 0.1 seconds, display data
            if(counter == 5){
                // Collect Average acceleration
                averageAcceleration.accelX /= counter;
                averageAcceleration.accelY /= counter;
                averageAcceleration.accelZ /= counter;

                accelerationThresholdFilter(&averageAcceleration);

                // Collect Distances
                struct distanceVals distance;
                distance = doubleIntegrationDistance(averageAcceleration);

                // Send Data over Serial
                //printf("Acceleration X: %.2f \t Acceleration Y: %.2f \t Acceleration Z: %.2f\n", acceleration.accelX, acceleration.accelY, acceleration.accelZ);
                //printf("Distance X: %.2f \t Distance Y: %.2f \t Distance Z: %.2f\n", distance.distX, distance.distY, distance.distZ);
                printf("%f,%f,%f,%f,%f,%f\n", averageAcceleration.accelX, averageAcceleration.accelY, averageAcceleration.accelZ, distance.distX, distance.distY, distance.distZ);

                // Reset Variables
                counter = 0;
                averageAcceleration.accelX = 0;
                averageAcceleration.accelY = 0;
                averageAcceleration.accelZ = 0;
            }
        }
    }
}

// Sensor fusion task -> uses sensor fusion to improve distance movements
// Garmin LIDAR v4 on X-axis, Ultrasonic sensors on Y and Z axes
static void sensor_fusion_task(void *arg){
    int evt;
    int counter = 0;

    // Initialize Sensor Value Readings
    struct distanceVals prevSensorDistances;
    prevSensorDistances.distX = collectGarminData();
    prevSensorDistances.distY = collectUltrasonicData1();

    // Initializes overall distance Value Readings
    struct distanceVals distance;
    distance.distX = 0;
    distance.distY = 0;
    distance.distZ = 0;

    // Initializes Acceleration Values
    struct accelerationVals tempAcceleration;
    struct accelerationVals averageAcceleration;
    averageAcceleration.accelX = 0;
    averageAcceleration.accelY = 0;
    averageAcceleration.accelZ = 0;


    while(1){
        // On the timer, collect data
        if (xQueueReceive(timer_queue, &evt, portMAX_DELAY)) {

            // Collect Acceleration Data
            tempAcceleration = getAccel();

            averageAcceleration.accelX += tempAcceleration.accelX;
            averageAcceleration.accelY += tempAcceleration.accelY;
            averageAcceleration.accelZ += tempAcceleration.accelZ;

            // Increment counter
            counter++;

            // Every second, display data
            if(counter == 5){

                // Collect Average acceleration
                averageAcceleration.accelX /= counter;
                averageAcceleration.accelY /= counter;
                averageAcceleration.accelZ /= counter;

                accelerationThresholdFilter(&averageAcceleration);

                // Collect Velocity Values
                struct velocityVals velocity;
                velocity = singleIntegrationVelocity(averageAcceleration);

                uint16_t currentXdist = collectGarminData();
                float currentYdist = collectUltrasonicData1();

                // If Velocity in any direction is greater than 0.5 m/s, use sensor data to determine distance moved
                if(fabs(velocity.velX) > VelocityThreshold){
                    distance.distX -= (currentXdist - prevSensorDistances.distX);
                }
                if(fabs(velocity.velY) > VelocityThreshold){
                    distance.distY -= (currentYdist - prevSensorDistances.distY);
                }

                // Set previous sensor distances
                prevSensorDistances.distX = currentXdist;
                prevSensorDistances.distY = currentYdist;

                //printf("Acceleration X: %.2f \t Acceleration Y: %.2f \t Acceleration Z: %.2f\n", acceleration.accelX, acceleration.accelY, acceleration.accelZ);
                //printf("Distance X: %.2f \t Distance Y: %.2f \t Distance Z: %.2f\n", distance.distX, distance.distY, distance.distZ);

                printf("%f,%f,%f,%f,%f,%f\n", averageAcceleration.accelX, averageAcceleration.accelY, averageAcceleration.accelZ, distance.distX, distance.distY, distance.distZ);

                // Reset Variables
                counter = 0;
                averageAcceleration.accelX = 0;
                averageAcceleration.accelY = 0;
                averageAcceleration.accelZ = 0;
            }
        }
    }
}


void app_main(void)
{
    // Initiates the timer queue
    timer_queue = xQueueCreate(10, sizeof(int));
    timer_init();

    // Initializes I2C communication with alphanumeric Display
    i2c_master_init();
    i2c_scanner();

    // Initializes Accelerometer and Ultrasonic Sensor (I don't think Garmin LIDAR v4 needs initialization)
    initializeADXL343();
    initializeUltrasonic();


    // Based on the mode set, output the correlated data to the JavaScript Node
    switch (mode) {
        // Double Integration Case
        case doubleIntegration:
            xTaskCreate(double_integration_task, "double_integration_task", 2048, NULL, 5, NULL);
            break;

        // If Sensor Fusion is enabled, initialize sensor fusion variables
        case sensorFusion:
            xTaskCreate(sensor_fusion_task, "sensor_fusion_task", 2048, NULL, 5, NULL);
            break;

        // If Sensor Fusion is enabled, initialize sensor fusion variables
        case phoneData:
            // Load data from all the phone files

            break;
        default:
            // Code
    }
}
