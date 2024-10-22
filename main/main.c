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

#define DATA_GPIO 38

#define TAG "main"

#define R 200
#define G 200
#define B 200

#include "icm20948-i2c-lib.h"
#include "sim_functions.h"

static led_strip_handle_t led_strip;

typedef struct vector {
    float x;
    float y;
    float z;
};

static void populate_matrix(struct Pixel pixel_array[]) {
    // Knuth algorithm
    int numbersSelected = 0;
    int rangeIndex;

    for (rangeIndex = 0; rangeIndex < MAX_LEDS && numbersSelected < NUM_SIM; ++rangeIndex) {
        int remainingNumbers = MAX_LEDS - rangeIndex;
        int remainingSelections = NUM_SIM - numbersSelected;

        if (esp_random() % remainingNumbers < remainingSelections) {
            // Select the current number
            ESP_LOGI(TAG, "selected %d", rangeIndex);
            numbersSelected++;
            pixel_array[rangeIndex].value = true;
        }
    }
    assert(numbersSelected == NUM_SIM);
}

static void update_pixel_data(struct Pixel pixel_array[], led_strip_handle_t led_strip) {
    for (int i = 0; i < MAX_LEDS; i++) {
        if (pixel_array[i].value == true) {
            led_strip_set_pixel(led_strip, i, R, G, B);
            // pixel_array[i].value = 0;
        } else {
            led_strip_set_pixel(led_strip, i, 0, 0, 0);
        }
    }
}

static struct vector get_unit_vector(i2c_master_dev_handle_t mag_handle) {
    struct vector unit_vector;
    struct magnetometer_result sensor_data = read_magnetometer(mag_handle);
    float magnitude = sqrt(sensor_data.x * sensor_data.x + sensor_data.y * sensor_data.y);

    unit_vector.x = sensor_data.x / magnitude;
    unit_vector.y = sensor_data.y / magnitude;

    return unit_vector;
}

static int angle_from_dot_product(struct vector base_vector, struct vector new_vector) {
    int angle = acos(base_vector.x * new_vector.x + base_vector.y * new_vector.y);
    return angle;
}

// No gyro library right now, hardcoding this for now
// static int get_angle(i2c_master_dev_handle_t mag_handle) {
//     struct magnetometer_result sensor_data = read_magnetometer(mag_handle);

//     // return new_angle;
// }
// static int get_angle(int angle) {

//     int new_angle;
//     if (angle <= 330) {
//         new_angle = angle + 15;
//     } else {
//         new_angle = 0;
//     }
//     return new_angle;
// }

static void configure_led_strip(void) {
    /* LED strip initialization with the GPIO and pixels number*/
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

void app_main(void) {

    // 1 sec startup time to let sensor start (idk if this does anything :)
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    int cur_angle;
    struct Pixel pixel_array[MAX_LEDS];

    configure_led_strip();
    configure_pixels(pixel_array);
    populate_matrix(pixel_array);

    // initiate device handlers
    i2c_master_dev_handle_t dev_handle = configure_dev_i2c();
    i2c_master_dev_handle_t mag_handle = configure_mag_i2c();
    check_sensor(dev_handle);
    // get_angle(mag_handle);
    struct vector base_unit_vector = get_unit_vector(mag_handle);
    ESP_LOGI(TAG, "Base vector cordinates are X: %f, Y: %f", base_unit_vector.x, base_unit_vector.y);

    while (true) {
        // for (int i = 0; i < 10; i++) {
        struct vector cur_unit_vector = get_unit_vector(mag_handle);
        ESP_LOGI(TAG, "new vector cordinates are X: %f, Y: %f", cur_unit_vector.x, cur_unit_vector.y);
        cur_angle = angle_from_dot_product(base_unit_vector, cur_unit_vector) + 90;

        run_sim(pixel_array, cur_angle);

        led_strip_clear(led_strip);
        update_pixel_data(pixel_array, led_strip);
        led_strip_refresh(led_strip);
        ESP_LOGI(TAG, "Angle changed to: %d", cur_angle);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        // }
        // cur_angle = get_angle(cur_angle);
    }
}
