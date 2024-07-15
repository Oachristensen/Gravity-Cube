#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/rmt_types.h"
#include "driver/rmt_tx.h"
#include "led_strip.h"
#include <rom/ets_sys.h>
#include "freertos/task.h"
#include <stdint.h> 
#include "esp_random.h"
#include "sim_functions.h"
 


#define DATA_GPIO 38


#define TAG "main"

#define R 5
#define G 0
#define B 0

static led_strip_handle_t led_strip;




static void populate_matrix(struct Pixel pixel_array[]) {
    //Knuth algorithm
    int numbersSelected = 0;
    int rangeIndex;

    for (rangeIndex = 0; rangeIndex < MAX_LEDS && numbersSelected < NUM_SIM; ++rangeIndex) {
        int remainingNumbers = MAX_LEDS - rangeIndex;
        int remainingSelections = NUM_SIM - numbersSelected;


        if (esp_random()% remainingNumbers  < remainingSelections) {
            // Select the current number
            ESP_LOGI(TAG, "selected %d", rangeIndex);
            numbersSelected++;
            pixel_array[rangeIndex].value = true;
        }
    }
    assert(numbersSelected == NUM_SIM);
}



static void update_pixel_data(struct Pixel pixel_array[], led_strip_handle_t led_strip) {
    for (int i = 0; i< MAX_LEDS; i++) {
        if (pixel_array[i].value == true) {
            led_strip_set_pixel(led_strip, i, R, G, B);
            // pixel_array[i].value = 0;
        }
        else {
            led_strip_set_pixel(led_strip, i, 0, 0, 0);
        }
    }
}



// static struct SpatialData check_pixel(int x, int y) {
//     for (int i = 0; i < MAX_LEDS; i++) {

//         if (i == index_from_cords(x,y)) {
//             continue;
//         }
//         // checking right
//         if (pixel_array[i].x != 0) {
//             if 
//         }
//     }
// }







static void configure_led_strip(void)
{
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

void app_main(void)
{
    while(true) {
        struct Pixel pixel_array[MAX_LEDS];
        configure_led_strip();
        configure_pixels(pixel_array);
        populate_matrix(pixel_array);
        for (int i = 0; i < 50; i++) {
            vTaskDelay(200 / portTICK_PERIOD_MS);
            run_sim(pixel_array);
            led_strip_clear(led_strip);
            update_pixel_data(pixel_array, led_strip);
            led_strip_refresh(led_strip);
        }
    }
}
