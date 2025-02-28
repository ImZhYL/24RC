#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "gpio_table.h"
#include "my_udp_client.h"
#include "spi_trans.h"
static const uint32_t BUFFER_SIZE = 40*1024; // IMAGE_SIZE;
// #define IMAGE_SIZE_GRAYSCALE_QVGA       320*240
// #define IMAGE_SIZE_GRAYSCALE_QQVGA      160*120  
// #define IMAGE_SIZE_GRAYSCALE_QQQVGA     80*60 
// #define IMAGE_SIZE                      IMAGE_SIZE_GRAYSCALE_QQQVGA
#define RX_BUF_SIZE 40000
static const char *TAG = "SPI Slave";
uint32_t a=BUFFER_SIZE;

WORD_ALIGNED_ATTR char *spiRecvDataPtr = NULL; //四字节对齐

/**
 * @description: SPI初始化
 * @param {*}
 * @return {*}
 */
void spiInit(void)
{
    esp_err_t ret;
    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = spi_mosi_pin,
        //.miso_io_num = spi_miso_io,
        .sclk_io_num = spi_clk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = RX_BUF_SIZE,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = 1,//模式0，1
        .spics_io_num = spi_cs_pin,
        .queue_size = 3,
        .flags = 0,
    };
    // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(spi_mosi_pin, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(spi_clk_pin, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(spi_cs_pin, GPIO_PULLUP_ONLY);
    // Initialize SPI slave interface
    ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO); // SPI DMA传输
    assert(ret == ESP_OK);
}

/**
 * @description: spi接收任务
 * @param {void} *pvParameters
 * @return {*}
 */
void spi_recv_task(void *pvParameters)
{
    spiInit();
    spiRecvDataPtr = (char *)malloc(RX_BUF_SIZE);
    spi_slave_transaction_t t;
    /* 补充代码*/
}



