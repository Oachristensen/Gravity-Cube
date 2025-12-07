
// header def
#ifndef icm20948_SPI_LIB_H
#define icm20948_SPI_LIB_H

#include "driver/spi_master.h"
#include "esp_log.h"

#include "string.h"

#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK 12
#define PIN_NUM_CS 5

#define ACCEL_XOUT_H 0x2D
#define ACCEL_XOUT_L 0x2E
#define ACCEL_YOUT_H 0x2F
#define ACCEL_YOUT_L 0x30
#define ACCEL_ZOUT_H 0x31
#define ACCEL_ZOUT_L 0x32
#define PWR_MGMT_1 0x06
#define ACCEL_CONFIG 0x14

#define REG_BANK_SEL 0x7F

#define USER_CTRL 0x03
#define I2C_IF_DIS_BIT 0x10

#define READ 0x80
#define WRITE 0x7F

#define FNTAG "icm20948-spi-lib"

#define WHO_AM_I_REG 0x00

typedef struct sensor_result {
    esp_err_t status;
    int x;
    int y;
    int z;
};

// TODO RE COMMENT AND DOCUMENT THIS, STILL A WARZONE FROM TRYING TO FIX A HORRIBLE BUG

// Write one register (2 bytes out, no bytes in)
esp_err_t icm20948_write_reg(spi_device_handle_t dev,
                             uint8_t reg,
                             uint8_t val);

esp_err_t icm20948_read_reg(spi_device_handle_t dev,
                            uint8_t reg,
                            uint8_t *out);

spi_device_handle_t configure_icm20948_spi();

esp_err_t icm20948_spi_read_burst(spi_device_handle_t icm_handle, uint8_t start_reg, uint8_t *data, size_t len);

struct sensor_result read_accelerometer_spi(spi_device_handle_t icm_handle);

#endif