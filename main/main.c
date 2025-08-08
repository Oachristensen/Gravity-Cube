#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_types.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include <math.h>
#include <rom/ets_sys.h>
#include <stdint.h>
#include <stdio.h>


#define TAG "main"

//CONFIG (Magic numbers)
// Set to your LED panel GPIO
#define DATA_GPIO 37
// RGB for the LEDS
#define R 10
#define G 10
#define B 100
// Delay between sim cycles (gets flickery below 25)
#define DELAY 35
//
#define MAIN_DEBUG false


#define MAX_LEDS 384

typedef struct my_vector {
    float x;
    float y;
    float z;
    float magnitude;
};

#include "icm20948-spi-lib.h"
#include "sim_functions.h"

static led_strip_handle_t led_strip;

#include "panel_data.h"

static void populate_matrix(struct Pixel pixel_array[]) {
    for (int i = 0; i <= NUM_SIM; i++) {
        pixel_array[i].value = true;
    }
}
static void update_pixel_data(struct Pixel pixel_array[], led_strip_handle_t led_strip) {
    int counter = 0;
    for (int i = 0; i < MAX_PIXELS; i++) {
        if (pixel_array[i].value == true) {
            int x = pixel_array[i].x;
            int y = pixel_array[i].y;
            int z = pixel_array[i].z;
            draw_panels(x, y, z, led_strip);
            counter++;
        }
    }
    if (MAIN_DEBUG) {
        ESP_LOGI(TAG, "Total Pixels active: %d", counter);
    }
}
// TODO Calibrate sensor data, (needs angle modification depending on esp orientation)
// Reads accelerometer and normalizes data into a 3d unit vector
static struct my_vector get_unit_vector(spi_device_handle_t icm_handle) {
    struct my_vector unit_vector;
    struct sensor_result sensor_data = read_accelerometer_spi(icm_handle);

    if (MAIN_DEBUG) {

        ESP_LOGI(TAG, "Raw sensor data X: %d, Y: %d, Z: %d Status: %s", sensor_data.x, sensor_data.y, sensor_data.z, esp_err_to_name(sensor_data.status));
    }
    float magnitude = sqrt((sensor_data.x * sensor_data.x) + (sensor_data.y * sensor_data.y) + (sensor_data.z * sensor_data.z));

    unit_vector.magnitude = magnitude;
    if (magnitude != 0) {
        // Changing some things because of sensor orientation
        unit_vector.x = sensor_data.z / magnitude;
        unit_vector.y = sensor_data.y / magnitude;
        unit_vector.z = sensor_data.x / magnitude;

    } else {
        // TODO proper error handling here, good monitoring sign though
        ESP_LOGI(TAG, "SOMETHING WENT WRONG");
    }
    return unit_vector;
}

static void configure_led_strip(void) {
    /* LED strip initialization with the GPIO and pixels number*/
    if (MAIN_DEBUG) {

        ESP_LOGI(TAG, "Max leds: %d", MAX_LEDS);
    }
    led_strip_config_t strip_config = {
        .led_model = LED_MODEL_WS2812,
        .strip_gpio_num = DATA_GPIO,
        .max_leds = MAX_LEDS, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

static void clear_led_strip(led_strip_handle_t led_strip) {
    for (int i = 0; i < MAX_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }
}

void app_main(void) {

    // 1 sec startup time to let sensor start (I think this does something :)
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    struct Pixel *pixel_array = malloc(MAX_PIXELS * sizeof(struct Pixel));
    if (!pixel_array) {
        ESP_LOGE(FN_TAG, "Memory allocation failed!");
        return;
    }

    init_panel_lookup();

    configure_led_strip();
    configure_pixels(pixel_array);
    populate_matrix(pixel_array);

    // initiate device handlers
    spi_device_handle_t icm_handle = configure_icm20948_spi();

    // check_sensor(icm_handle);

    struct my_vector last_vector;
    struct my_vector unit_vector = get_unit_vector(icm_handle);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    while (true) {
        unit_vector = get_unit_vector(icm_handle);

        last_vector = unit_vector;
        run_sim(pixel_array, last_vector);
        if (MAIN_DEBUG) {
            ESP_LOGI(TAG, "x: %f, y: %f z: %f", unit_vector.x, unit_vector.y, unit_vector.z);
        }
        clear_led_strip(led_strip);
        update_pixel_data(pixel_array, led_strip);
        led_strip_refresh(led_strip);
        vTaskDelay(DELAY / portTICK_PERIOD_MS);
    }
    free(pixel_array);
}
