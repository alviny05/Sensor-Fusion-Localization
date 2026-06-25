/*
This file contains the code for initializing and collecting data from the Ultrasonic Sensor
through the ADC Pin.
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#include "ultrasonic.h"

// Global Variables for the file
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel1 = ADC_CHANNEL_6;     //GPIO34 (A2) if ADC1, GPIO14 if ADC2
static const adc_channel_t channel2 = ADC_CHANNEL_0;     //TODO - Check if right: GPIO38 (A4) if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_12;
static const adc_unit_t unit = ADC_UNIT_1;


// Functions used internally by the ultrasonic sensor
static void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}



// Global Functions
void initializeUltrasonic(void){
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel1, atten);
        adc1_config_channel_atten(channel2, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel1, atten);
        adc2_config_channel_atten((adc2_channel_t)channel2, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

// Function to collect Ultrasonic Sensor Data from the sensor connected to A2
float collectUltrasonicData1(void){

    // Variable Definitions
    uint32_t adc_reading;
    uint32_t voltage;
    float distance;

    // Read ADC data
    adc_reading = adc1_get_raw((adc1_channel_t)channel1);

    // Conver ADC reading to Voltage
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    // Convert mV to distance: Since we have a 3.3V power supply, it's 6.4mV per inch
    distance = voltage / 6.4;

    // Convert inches to centimeters
    distance *= 2.54;

    return distance;
}

// Function to collect Ultrasonic Sensor Data from the sensor connected to A4
float collectUltrasonicData2(void){

    // Variable Definitions
    uint32_t adc_reading;
    uint32_t voltage;
    float distance;

    // Read ADC data
    adc_reading = adc1_get_raw((adc1_channel_t)channel2);

    // Conver ADC reading to Voltage
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    // Convert mV to distance: Since we have a 3.3V power supply, it's 6.4mV per inch
    distance = voltage / 6.4;

    // Convert inches to centimeters
    distance *= 2.54;

    return distance;
}

