#include <math.h>
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_table.h"
#include "device_manager.h"


const char *Dev_Tag = "Device Manager";
void device_manager_init(void){
    gpio_config_t dev_mgr;
    dev_mgr.pin_bit_mask = (1ULL << openmv_rst_pin) | (1ULL << opa_lx_en_pin) | (1ULL << opa_ly_en_pin) | (1ULL << opa_rx_en_pin) | (1ULL << opa_ry_en_pin) | (1ULL << clamp_a_en2_pin) | (1ULL << clamp_b_en2_pin) ;
    dev_mgr.mode = GPIO_MODE_OUTPUT;
    //dev_mgr.pull_up_en = GPIO_PULLUP_ENABLE;
    dev_mgr.pull_down_en = GPIO_PULLDOWN_ENABLE;
    dev_mgr.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&dev_mgr);
    ESP_LOGI(Dev_Tag, "Successfully initialize device manager");
    gpio_set_level(openmv_rst_pin, 0);
    gpio_set_level(opa_lx_en_pin, 0);
    gpio_set_level(opa_ly_en_pin, 0);
    gpio_set_level(opa_rx_en_pin, 0);
    gpio_set_level(opa_ry_en_pin, 0);
    gpio_set_level(clamp_a_en2_pin, 0);
    gpio_set_level(clamp_b_en2_pin, 0);
}

inline void resume_openmv(void){
    gpio_set_level(openmv_rst_pin, 1);
    ESP_LOGI(Dev_Tag, "Successfully resume openmv");
}
inline void suspend_openmv(void){
    gpio_set_level(openmv_rst_pin, 0);
    ESP_LOGI(Dev_Tag, "Successfully suspend openmv");
}

inline void suspend_walk_opas(void){
    gpio_set_level(opa_lx_en_pin, 0);
    gpio_set_level(opa_ly_en_pin, 0);
    gpio_set_level(opa_rx_en_pin, 0);
    gpio_set_level(opa_ry_en_pin, 0);
    ESP_LOGI(Dev_Tag, "Successfully suspend walking opas");
}
inline void suspend_clamp_opas(void){
    gpio_set_level(clamp_a_en2_pin, 0);
    gpio_set_level(clamp_b_en2_pin, 0);
    ESP_LOGI(Dev_Tag, "Successfully suspend clamp opas");
}
inline void resume_walk_opas(void){
    gpio_set_level(opa_lx_en_pin, 1);
    gpio_set_level(opa_ly_en_pin, 1);
    gpio_set_level(opa_rx_en_pin, 1);
    gpio_set_level(opa_ry_en_pin, 1);
    ESP_LOGI(Dev_Tag, "Successfully resume walking opas");
}
inline void resume_clamp_opas(void){
    gpio_set_level(clamp_a_en2_pin, 1);
    gpio_set_level(clamp_b_en2_pin, 1);
    ESP_LOGI(Dev_Tag, "Successfully resume clamp opas");
}