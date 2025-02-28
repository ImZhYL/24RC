#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_event.h"

// 私有头文件

#include "my_tcp_client.h"
#include "pwm_controller.h"
#include "boost_controller.h"
#include "uart_controller.h"
#include "wifi_connect.h"
#include "spi_trans.h"
#include "my_udp_client.h"
#include "led_controller.h"
#include "device_manager.h"
#include "gpio_table.h"
static const char *TAG = "work";

void app_main(void)
{
  // 初始化
  ESP_LOGI(TAG, "Init");
  //硬件使能引脚初始化
  device_manager_init();
  // PWM模块初始化
  boost_controller_init();
  change_voltage(40, max_voltage);
  pwm_controller_init();
  //开启任务
  uart_controller_init();
  wifi_init();
  xTaskCreate(uart_rx_task, "uart_rx_task", 8192, NULL, 6, NULL);
  xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, configMAX_PRIORITIES - 1, NULL);
  xTaskCreate(my_udp_client_task, "my_udp_client", 16384, NULL, 7, &udp_client_task);
  xTaskCreate(spi_recv_task, "spi_trans", 16384, NULL, 4, NULL);
  // led初始化
  led_init();
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    led_on();
    vTaskDelay(pdMS_TO_TICKS(1000));
    led_off();
  }
}