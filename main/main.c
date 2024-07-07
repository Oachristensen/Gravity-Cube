#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/rmt_types.h"
#include "driver/rmt_tx.h"
#include "led_strip.h"
#include <rom/ets_sys.h>
#include "freertos/task.h"
#include "esp_random.h"

#include <stdint.h> 
 


#define DATA_GPIO 16


#define X_SIZE 8
#define Y_SIZE 8

#define MAX_LEDS X_SIZE*Y_SIZE

#define NUM_SIM 32

#define TAG "main"

#define R 5
#define G 0
#define B 0



static led_strip_handle_t led_strip;

typedef struct Pixel
{
 int x;
 int y;
 int value;
};

struct Pixel pixel_array[MAX_LEDS];



static int index_from_cords(int x, int y) {
    return(x+(y*(Y_SIZE)));
}

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

static void configure_pixels(void) {
    for (int y = 0; y < Y_SIZE; y++) {
         for (int x = 0; x < X_SIZE; x++) {
            pixel_array[index_from_cords(x, y)].x = x;
            pixel_array[index_from_cords(x,y)].y = y;
            pixel_array[index_from_cords(x,y)].value = 0;

        }
    }
}
static void update_pixel_data(void) {
    for (int i = 0; i< MAX_LEDS; i++) {
        if (pixel_array[i].value == 1) {
            led_strip_set_pixel(led_strip, i, R, G, B);
            pixel_array[i].value = 0;
        }
    }
}

static void run_sim(void) {
    //Check pixel left
    //Check pixel right
    //Check pixel down
    bool left = true;
    bool right = true;
    bool down = true;
}



static void populate_matrix(void) {
    //Knuth algorithm
    int numbersSelected = 0;
    int rangeIndex;

    for (rangeIndex = 0; rangeIndex < MAX_LEDS && numbersSelected < NUM_SIM; ++rangeIndex) {
        int remainingNumbers = MAX_LEDS - rangeIndex;
        int remainingSelections = NUM_SIM - numbersSelected;


        if (esp_random()% remainingNumbers  < remainingSelections) {
            // Select the current number
            numbersSelected++;
            pixel_array[rangeIndex].value = 1;
        }
    }
    assert(numbersSelected == NUM_SIM);
}


void app_main(void)
{
    configure_led_strip();
    configure_pixels();

    
    while(true) {
        led_strip_clear(led_strip);
        populate_matrix();
        update_pixel_data();
        led_strip_refresh(led_strip);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
