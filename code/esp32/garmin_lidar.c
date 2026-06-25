/*
This file contains the code for initializing and collecting data from the Garmin LIDAR v4
Sensor through I2C Communication.
Justin Nascimento (U42983905) and Alvin Yan ()
*/
#include "garmin_lidar.h"

// Function to write to the Garmin V4 over SCL and SDA
uint16_t collectGarminData(){

    // Register values from the garmin
    uint8_t reg01;
    uint8_t reg10 = 0;
    uint8_t reg11 = 0;

    // Distance
    uint16_t distance;

    // I2C Command Handler
    i2c_cmd_handle_t cmd4;

    // Writes 0x04 to register 0x00
    cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, (GARMIN_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);     // Indicate we're writing
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);                       // Sends start address -> 0x00
    i2c_master_write_byte(cmd4, 0x04, ACK_CHECK_EN);                                // Writes 0x04 to register 1
    i2c_master_stop(cmd4);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd4);

    // Reads 0x01 until LSB is 0
    do {
        cmd4 = i2c_cmd_link_create();

        i2c_master_start(cmd4);
        i2c_master_write_byte(cmd4, (GARMIN_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd4, 0x01, ACK_CHECK_EN);

        i2c_master_start(cmd4);
        i2c_master_write_byte(cmd4, (GARMIN_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
        i2c_master_read_byte(cmd4, &reg01, NACK_VAL);

        i2c_master_stop(cmd4);

        i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd4);

        vTaskDelay(10 / portTICK_PERIOD_MS);

    } while (reg01 & 0x01);


    // Reads 0x10 and 0x11 to get the data
    cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);

    // Indicates we intend to read from 0x01
    i2c_master_write_byte(cmd4, (GARMIN_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);       // Tell it we're writing to 0x10
    i2c_master_write_byte(cmd4, 0x10, ACK_CHECK_EN);                                // Write 0x10

    // Tactical restart to now indicate we're reading
    i2c_master_start(cmd4);  // Repeated start, to indicate we're reading
    i2c_master_write_byte(cmd4, (GARMIN_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd4, &reg10, ACK_VAL);
    i2c_master_read_byte(cmd4, &reg11, NACK_VAL);
    i2c_master_stop(cmd4);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd4);

    // Format the distance
    distance = (reg11 * 256) + reg10;

    return distance;

}
