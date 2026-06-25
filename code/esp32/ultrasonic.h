/*
This file contains the code for initializing and collecting data from the Ultrasonic Sensor
through the ADC Pin.
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

// Libraries
#include <stdio.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

// Defines
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   20          //Multisampling

// Function Prototypes
void initializeUltrasonic(void);
float collectUltrasonicData1(void);
float collectUltrasonicData2(void);

#endif