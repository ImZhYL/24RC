#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
//#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <sys/socket.h>
#include "my_udp_client.h"
#include "gpio_table.h"
// #include "string.h"
char *UDP_payload = "ESP32_sucessfully_connected";

static const char *TAG = "udp client";

char *udpSendDataPtr = NULL;
int udpSendDataLength = 0;
TaskHandle_t udp_client_task = NULL;
/**
 * @description:  udp client任务
 * @param {void} *pvParameters
 * @return {*}
 */
void my_udp_client_task(void *pvParameters)
{
    int addr_family = 0;
    int ip_protocol = 0;
    while (1)
    {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, UDP_PORT);
        sendto(sock, UDP_payload, strlen(UDP_payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        ESP_LOGI(TAG, "core:%d", xPortGetCoreID());
        while (1)
        {
            if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) != 0)
            {
                int err = sendto(sock, udpSendDataPtr, udpSendDataLength, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0)
                {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
        }
        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

