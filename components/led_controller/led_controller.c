#include <math.h>
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_table.h"
#include "led_controller.h"



/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2, ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */
static const char *TAG = "LED_controller";
void led_init(void){
    gpio_config_t led_flash;
    led_flash.pin_bit_mask = (1ULL <<led_pin) ;
    led_flash.mode = GPIO_MODE_OUTPUT;
    led_flash.pull_up_en = GPIO_PULLUP_ENABLE;
    led_flash.pull_down_en = GPIO_PULLDOWN_ENABLE;
    led_flash.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&led_flash);
    ESP_LOGI(TAG, "Successfully initialize LED");
    gpio_set_level(led_pin, 0);
    gpio_set_level(led_pin, 1);
    gpio_set_level(led_pin, 0);
    gpio_set_level(led_pin, 1);
    gpio_set_level(led_pin, 0);
}
void led_on(void){
    gpio_set_level(led_pin, 1);
}

void led_off(void){
    gpio_set_level(led_pin, 0);
}
