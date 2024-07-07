#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/rmt_types.h"
#include "driver/rmt_tx.h"
#include "led_strip.h"
#include <rom/ets_sys.h>
#include "freertos/task.h"
 


#define DATA_GPIO 16
#define MAX_LEDS 64
#define TAG "main"



static led_strip_handle_t led_strip;


static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
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

    static int index_from_cords(int x, int y) {
return 1;
    }

void app_main(void)
{
    configure_led();

    
    while(true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        led_strip_clear(led_strip);
        for (int index = 0; index < MAX_LEDS; index++) {
        led_strip_set_pixel(led_strip, index, 1, 4, 6);
        led_strip_refresh(led_strip);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        
    }
}
