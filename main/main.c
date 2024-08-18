#include "driver/gpio.h"
#include "esp_mac.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_types.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include <math.h>
#include "sim_functions.h"
#include <rom/ets_sys.h>
#include <stdint.h>
#include <stdio.h>


#define DATA_GPIO 38



#define TAG "main"

#define R 1
#define G 0
#define B 0

static led_strip_handle_t led_strip;

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

// No gyro library right now, hardcoding this for now
static int get_angle(int cur_angle) {
    int new_angle;
    if (cur_angle <= 330) {
    new_angle = cur_angle + 15;
    }
    else {
        new_angle = 0;
    }
    return new_angle;
}

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
    int cur_angle = 0;
    struct Pixel pixel_array[MAX_LEDS];
    configure_led_strip();
    configure_pixels(pixel_array);
    populate_matrix(pixel_array);

    while (true) {
        for (int i = 0; i < 10; i++) {
        run_sim(pixel_array, cur_angle);
        led_strip_clear(led_strip);
        update_pixel_data(pixel_array, led_strip);
        led_strip_refresh(led_strip);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        cur_angle = get_angle(cur_angle);

        
    }
}
