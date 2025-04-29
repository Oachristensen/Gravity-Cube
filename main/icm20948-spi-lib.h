
#include "driver/spi_master.h"
#include "esp_log.h"


//TODO remove this and change memset?
#include "string.h"

#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK 12
#define PIN_NUM_CS 10

#define ACCEL_XOUT_H 0x2D
#define ACCEL_XOUT_L 0xEh
#define ACCEL_YOUT_H 0x2F
#define ACCEL_YOUT_L 0x30
#define ACCEL_ZOUT_H 0x31
#define ACCEL_ZOUT_L 0x32
#define PWR_MGMT_1 0x06
#define ACCEL_CONFIG 0x14

#define READ 0x80
#define WRITE 0x7F

#define FNTAG "icm20948-spi-lib"

// ICM-20948 expects a read flag (bit 7 set) on read
#define WHO_AM_I_REG 0x00
#define WHO_AM_I_EXPECTED 0xEA

typedef struct sensor_result {
    esp_err_t status;
    int16_t x;
    int16_t y;
    int16_t z;
};
static struct sensor_result last_good;

// Takes in a device handle, reg address, and data to write to that address
esp_err_t icm20948_spi_write_register(spi_device_handle_t icm_handle, uint8_t reg, uint8_t val) {
    uint8_t tx[2] = {reg & 0x7F, val}; // clear MSB for write
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx,
        .rx_buffer = NULL,
    };
    return spi_device_transmit(icm_handle, &t);
}

// Takes in a device handle, reg address, and reads data from that reg
esp_err_t icm20948_spi_read_register(spi_device_handle_t icm_handle, uint8_t reg, uint8_t *out_val) {
    uint8_t tx[2] = {reg | 0x80, 0x00};
    uint8_t rx[2] = {0};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };
    esp_err_t ret = spi_device_transmit(icm_handle, &t);
    if (ret == ESP_OK) {
        *out_val = rx[1];
    }
    return ret;
}


spi_device_handle_t configure_icm20948_spi() {
    spi_device_handle_t icm_handle;
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
    };

    spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI3_HOST, &devcfg, &icm_handle);

    // Wake up and configure sensor
    icm20948_spi_write_register(icm_handle, PWR_MGMT_1, 0x00);   // Wake
    icm20948_spi_write_register(icm_handle, ACCEL_CONFIG, 0x01); // ±2g

    return icm_handle;
}


esp_err_t icm20948_spi_read_burst(spi_device_handle_t icm_handle, uint8_t start_reg, uint8_t *data, size_t len) {
    uint8_t tx[len + 1];
    uint8_t rx[len + 1];

    tx[0] = start_reg | 0x80; // Set read bit
    memset(&tx[1], 0, len);   // Fill rest with dummy bytes

    spi_transaction_t t = {
        .length = 8 * (len + 1),
        .tx_buffer = tx,
        .rx_buffer = rx,
    };
    esp_err_t ret = spi_device_transmit(icm_handle, &t);
    if (ret != ESP_OK)
        return ret;

    memcpy(data, &rx[1], len); // skip the first dummy byte
    return ESP_OK;
}

struct sensor_result read_accelerometer_spi(spi_device_handle_t icm_handle) {
    struct sensor_result data = {.status = ESP_OK};
    uint8_t raw[6];

    esp_err_t ret = icm20948_spi_read_burst(icm_handle, ACCEL_XOUT_H, raw, 6);
    if (ret == ESP_OK) {
        data.x = (int16_t)(raw[0] << 8 | raw[1]);
        data.y = (int16_t)(raw[2] << 8 | raw[3]);
        data.z = (int16_t)(raw[4] << 8 | raw[5]);
        last_good = data;
    } else {
        data = last_good;
        data.status = ret;
        ESP_LOGW(FNTAG, "SPI accel read failed, using last good");
    }
    return data;
}