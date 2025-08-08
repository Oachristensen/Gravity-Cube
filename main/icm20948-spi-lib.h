
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
static struct sensor_result last_good;

//TODO RE COMMENT AND DOCUMENT THIS, STILL A WARZONE FROM TRYING TO FIX A HORRIBLE BUG



// Write one register (2 bytes out, no bytes in)
static esp_err_t icm20948_write_reg(spi_device_handle_t dev,
                           uint8_t reg,
                           uint8_t val) {
    // Address byte must have MSB=0 for writes
    uint8_t addr = reg & 0x7F;

    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA, // use built-in tx_data[]
        .length = 16,                  // send addr + val (2 bytes ×8)
        .tx_data = {addr, val, 0, 0}   // [0]=addr, [1]=value
    };
    return spi_device_polling_transmit(dev, &t);
}

static esp_err_t icm20948_read_reg(spi_device_handle_t dev,
                          uint8_t reg,
                          uint8_t *out) {
    // MSB=1 for read
    uint8_t addr = reg | 0x80;

    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length = 16,   // send only the address byte
        .rxlength = 16, // read two bytes back
        .tx_data = {addr, 0x00, 0, 0}};

    esp_err_t err = spi_device_polling_transmit(dev, &t);
    if (err == ESP_OK) {
        *out = t.rx_data[1]; // the second byte is the register value
    }
    return err;
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
        .mode = 3,
        .cs_ena_pretrans = 4,  // 4 SPI clock cycles
        .cs_ena_posttrans = 4, // 4 SPI clock cycles
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &icm_handle));

    // ==== BANK 0: reset & wake ====
    icm20948_write_reg(icm_handle, REG_BANK_SEL, 0<<4);

    icm20948_write_reg(icm_handle, PWR_MGMT_1, 0x80);
    icm20948_write_reg(icm_handle, PWR_MGMT_1, 0x01);
    icm20948_write_reg(icm_handle, USER_CTRL, I2C_IF_DIS_BIT);

    // ==== BANK 2: accel config ====
    icm20948_write_reg(icm_handle, REG_BANK_SEL, 2 << 4);
    icm20948_write_reg(icm_handle, ACCEL_CONFIG, 0x00); // ±2g

    // ==== back to BANK 0 for WHO_AM_I ====
    vTaskDelay(10 / portTICK_PERIOD_MS);

    icm20948_write_reg(icm_handle, REG_BANK_SEL, 0 << 4);
    icm20948_write_reg(icm_handle, PWR_MGMT_1, 0x01);
    icm20948_write_reg(icm_handle, USER_CTRL, I2C_IF_DIS_BIT);

    uint8_t pwr = 0, usr = 0;
    // read back PWR_MGMT_1 (0x06)
    icm20948_read_reg(icm_handle, PWR_MGMT_1, &pwr);
    // read back USER_CTRL  (0x03)
    icm20948_read_reg(icm_handle, USER_CTRL, &usr);

    ESP_LOGI(FNTAG, "PWR_MGMT_1 = 0x%02X, USER_CTRL = 0x%02X", pwr, usr);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    uint8_t who;
    icm20948_write_reg(icm_handle, REG_BANK_SEL, 0 << 4);
    icm20948_read_reg(icm_handle, WHO_AM_I_REG, &who);
    ESP_LOGI(FNTAG, "WHO_AM_I = 0x%02X", who);
    ESP_LOGI(FNTAG, "Who am I: %d", who);
    return icm_handle;
}

esp_err_t icm20948_spi_read_burst(spi_device_handle_t icm_handle, uint8_t start_reg, uint8_t *data, size_t len) {
    uint8_t tx[len + 1];
    uint8_t rx[len + 1];

    tx[0] = start_reg | READ; // Set read bit
    memset(&tx[1], 0, len);   // Fill rest with dummy bytes

    spi_transaction_t t = {
        // .addr = start_reg,
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
    icm20948_write_reg(icm_handle, REG_BANK_SEL, 0 << 4);

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