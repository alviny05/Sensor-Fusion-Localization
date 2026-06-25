/*
This file contains the code for initializing and collecting data from the Garmin LIDAR v4
Sensor through I2C Communication.
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#ifndef GARMIN_LIDAR_H
#define GARMIN_LIDAR_H

#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_functions.h"

// Definitions for Alphanumeric Display Integration
// 14-Segment Display
#define GARMIN_ADDR                        0x62 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd

// Function Prototypes
uint16_t collectGarminData(void);

#endif
