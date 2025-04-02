#include "driver/i2c_master.h"
#include "esp_log.h"
#include <stdio.h>

// Defining registers

// Magnetometer
#define WHO_AM_I 0x00
#define MAG_X_L 0x11
#define MAG_X_H 0x12
#define MAG_Y_L 0x13
#define MAG_Y_H 0x14
#define MAG_Z_L 0x15
#define MAG_Z_H 0x16

// Accelerometer
#define ACCEL_XOUT_H 0x2D
#define ACCEL_XOUT_L 0xEh
#define ACCEL_YOUT_H 0x2F
#define ACCEL_YOUT_L 0x30
#define ACCEL_ZOUT_H 0x31
#define ACCEL_ZOUT_L 0x32

#define RST 0x06
#define CNTL2 0x31
#define CNTL3 0x32

// I2C
#define SDA 8
#define SCL 9

// Mag I2C
#define ASDA 10
#define ASCL 11
// needs A0 connected to vcc
#define DEV_ADDRESS 0x69

#define DATA_LENGTH 100

#define FNTAG "icm20948-lib"


// TODO: MAKE PROPER ERROR HANDLING

typedef struct sensor_result {
    esp_err_t status;
    int16_t x;
    int16_t y;
    int16_t z;
};
struct sensor_result last_good;

// Startup the main device bus and run some configuration steps
i2c_master_dev_handle_t configure_dev_i2c() {
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1,
        .scl_io_num = SCL,
        .sda_io_num = SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DEV_ADDRESS,
        .scl_speed_hz = 400000,
    };
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    // Resets the sensors power settings
    uint8_t rst[2] = {RST, 1};
    i2c_master_transmit(dev_handle, rst, sizeof(rst), -1);

    // Wake up device and enable accelerometer
    uint8_t pwr_mgmt[2] = {0x06, 0x00}; // PWR_MGMT_1, reset sleep mode
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, pwr_mgmt, 2, -1));

    uint8_t accel_config[2] = {0x14, 0x01}; // ACCEL_CONFIG, default range ±2g
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, accel_config, 2, -1));

    return dev_handle;
}
//! NO LONGER USED
// Startup the magnetometer device bus and run some configuration steps
i2c_master_dev_handle_t configure_mag_i2c() {
    i2c_master_bus_config_t mag_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1,
        .scl_io_num = ASCL,
        .sda_io_num = ASDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t mag_bus_handle;

    ESP_ERROR_CHECK(i2c_new_master_bus(&mag_bus_config, &mag_bus_handle));

    i2c_device_config_t mag_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x0C,
        .scl_speed_hz = 400000,
    };
    i2c_master_dev_handle_t mag_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(mag_bus_handle, &mag_cfg, &mag_handle));

    // reset magenetometer
    uint8_t cntl3[2] = {CNTL3, 1};
    i2c_master_transmit(mag_handle, cntl3, sizeof(cntl3), -1);

    // set magnetometer to continuous measurement mode
    uint8_t cntl2[2] = {CNTL2, 0x02};
    esp_err_t ret = i2c_master_transmit(mag_handle, cntl2, sizeof(cntl2), -1);

    return mag_handle;
}

// Checks that the sensor is connected by checking its WHO_AM_I_REGISER
void check_sensor(i2c_master_dev_handle_t dev_handle) {
    uint8_t who_am_i_write[1] = {WHO_AM_I};
    uint8_t readbuffer[DATA_LENGTH];
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, who_am_i_write, sizeof(who_am_i_write), readbuffer, DATA_LENGTH, -1);
    if (ret == ESP_OK) {
        ESP_LOGI(FNTAG, "WHO_AM_I: %d", readbuffer[0]);
    } else {
        ESP_LOGI(FNTAG, "Sensor not working, check your wiring and pins ERROR: %s", esp_err_to_name(ret));
    }
}

//! NO LONGER USED
// Reads magnetometer from i2c and returns raw x, y, z values
struct sensor_result read_magnetometer(i2c_master_dev_handle_t dev_handle) {
    struct sensor_result data;
    data.status = ESP_OK;

    uint8_t read[100];

    uint8_t starting_address[1] = {MAG_X_L}; // Will be read seqentially MAG_XL, MAG_X_H, MAG_Y_L, MAG_Y_H, MAG_Z_L, MAG_Z_H (0x11-0x16)
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, starting_address, sizeof(starting_address), read, DATA_LENGTH, -1);
    if (ret == ESP_OK) {
        data.x = (int16_t)(read[0] | (read[1] << 8));
        data.y = (int16_t)(read[2] | (read[3] << 8));
        data.z = (int16_t)(read[4] | (read[5] << 8));
    } else {
        data.status = ret;
        data.x = 0;
        data.y = 0;
        data.z = 0;
        ESP_LOGI(TAG, "magnometer read failed");
    }

    return data;
}

// Reads accelerometer from i2c and returns raw x, y, z values
struct sensor_result read_accelerometer(i2c_master_dev_handle_t dev_handle) {
    struct sensor_result data;
    data.status = ESP_OK;

    uint8_t read[100];

    uint8_t starting_address[1] = {ACCEL_XOUT_H}; // Will be read seqentially MAG_XL, MAG_X_H, MAG_Y_L, MAG_Y_H, MAG_Z_L, MAG_Z_H (0x11-0x16)

    esp_err_t ret = i2c_master_transmit_receive(dev_handle, starting_address, sizeof(starting_address), read, DATA_LENGTH, 50);
    if (ret == ESP_OK) {
        data.x = (int16_t)(read[0] << 8 | (read[1]));
        data.y = (int16_t)(read[2] << 8 | (read[3]));
        data.z = (int16_t)(read[4] << 8 | (read[5]));
        last_good = data;
    }
    //accel often fails due to voltage drops, this is a 'temporary fix' :)
    if (ret != ESP_OK) {
        ESP_LOGE(FNTAG, "I2C failed: %s, resetting", esp_err_to_name(ret));
        i2c_master_bus_reset(dev_handle);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        return last_good;
    }
    return data;
}
