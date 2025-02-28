#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
// #include "esp_event.h"
#include "protocol_examples_common.h"
#include "addr_from_stdin.h"
// #include "lwip/err.h"
// #include "lwip/sockets.h"
#include "my_tcp_client.h"
#include "cmd_decoder.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h> // struct addrinfo
#include <arpa/inet.h>
#include "gpio_table.h"

// #include "string.h"

// #define HOST_IP_ADDR "192.168.137.1" // IP地址
// #define PORT 6666                    // 端口号

static const char *TAG = "tcp_client";
char *tcp_payload = "ESP32_sucessfully_connected/I";

bool bshutdowm = 0; // 关闭的标记
int sock_id = 0;    // socket的ID保存为全局变量
/******************************************
函数：tcp发送信息
********************************************/
void tcp_sendData(char *SendMsg)
{
    int err1 = send(sock_id, SendMsg, strlen(SendMsg), 0);
    if (err1 < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}

/******************************************
函数：tcp/socket调用函数
********************************************/
void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1)
    {
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(TCP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        sock_id = sock;
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, TCP_PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");
        tcp_sendData(tcp_payload);
        while (1)
        {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // 判断接收数据出错
            if (len < 0)
            {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // 处理接收到的数据
            else
            {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if (decode_and_run(rx_buffer, host_computer_id) != host_computer_id)
                {
                    char *data = "Being Blocked";
                    int err = send(sock, data, strlen(data), 0);
                    if (err < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        break;
                    }
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
}

// /******************************************
// 函数：tcp通讯初始化
// ********************************************/
// void wifi_init()

// {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     ESP_ERROR_CHECK(example_connect());
// }
