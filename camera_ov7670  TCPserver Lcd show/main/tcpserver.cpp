/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS products only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "lwip/api.h"
#include "camera.h"
#include "bitmap.h"
#include "iot_lcd.h"
#include "iot_tcp.h"
#include "iot_wifi_conn.h"
#include "app_camera.h"
#include "nvs_flash.h"
#include "esp_wifi_internal.h"
#include "string.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "iot_tcp.h"

#define SERVER_PORT             8080
#define SERVER_MAX_CONNECTION   20

static const char* TAG_SRV = "TCP_SRV";
static EventGroupHandle_t wifi_event_group;
volatile static uint32_t * * currFbPtr __attribute__ ((aligned(4))) = NULL;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            break;
        default:
            break;
    }
    return ESP_OK;
}

void initialise_wifi(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t wifi_config;
    memcpy(wifi_config.ap.ssid, "123456789", sizeof("123456789"));
    memcpy(wifi_config.ap.password, "123456789", sizeof("123456789"));
    wifi_config.ap.ssid_len = strlen("123456789");
    wifi_config.ap.max_connection = 1;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    esp_wifi_start();
}

void initialise_buffer()
{
    currFbPtr= (volatile uint32_t **)malloc(sizeof(uint32_t *) * CAMERA_CACHE_NUM);

    ESP_LOGI(TAG_SRV, "get free size of 32BIT heap : %d\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));

    for(int i = 0; i < CAMERA_CACHE_NUM; i++){
        currFbPtr[i] = (volatile uint32_t *) heap_caps_malloc(320 * 240 * 2, MALLOC_CAP_32BIT);
        ESP_LOGI(TAG_SRV, "framebuffer address is:%p\n", currFbPtr[i]);
    }
}

extern "C" void tcp_client_obj_test()
{
   CTcpConn client;
   const char* data = "test1234567";
   uint8_t recv_buf[100];

   tcpip_adapter_ip_info_t ipconfig;
   tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipconfig);

   ESP_LOGI(TAG_SRV, "TCP connect...");
   ESP_LOGI(TAG_SRV, "connect to: %d.%d.%d.%d(%d)", ((uint8_t*)&ipconfig.ip.addr)[0], ((uint8_t*)&ipconfig.ip.addr)[1],
                                                     ((uint8_t*)&ipconfig.ip.addr)[2], ((uint8_t*)&ipconfig.ip.addr)[3], SERVER_PORT);

   if (client.Connect(ipconfig.ip.addr, SERVER_PORT) < 0) {
       ESP_LOGI(TAG_SRV, "fail to connect...");
       vTaskDelay(5000 / portTICK_PERIOD_MS);
   }

   while(1) {
       if (client.Write((const uint8_t*)data, strlen(data)) < 0) {
           vTaskDelay(5000 / portTICK_PERIOD_MS);
       }
   }
}

void tcp_server_handle(void* arg)
{
    static uint8_t i = 0;
    uint8_t* data;
    uint32_t data1;
    uint32_t length = 320*240*2;
//    CTcpConn *conn = (CTcpConn*) arg;
//    conn->SetTimeout(10);
    int sockfd = (int)arg;
    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 10;
    receiving_timeout.tv_usec = 0;
    data1 = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout));
    ESP_LOGI(TAG_SRV, "setsockopt %d", data1);

    while (1) {
        if (sockfd > 0) {
            data = (uint8_t *)currFbPtr[i % CAMERA_CACHE_NUM];

            length = 320*240*2;
            while(length > 0){
                data1 = recv(sockfd, (uint8_t *) data, length, MSG_WAITALL);
//                ESP_LOGI(TAG_SRV, "recv %d num", data1);
                data += data1;
                length -= data1;
            }

//            for(int j = 0;j < 1200; j++){
////                data1 = recv(sockfd, data, 10000, 0);
////                data1 = read(sockfd, data, 10240);
//                length = 128;
//                while(length > 0)
//                {
//                    data1 = recv(sockfd, data, length, MSG_WAITALL);
//                    length -= data1;
//                }
//                data += 128;
////                ESP_LOGI(TAG_SRV, "read recv %d num: %d", j, data1);
//            }
            i = i % CAMERA_CACHE_NUM;
            queue_send((uint16_t *)currFbPtr[i++]);
        }
    }
}

void tcp_server_task(void *pvParameters)
{
//   CTcpServer server;
//   server.Listen(SERVER_PORT, SERVER_MAX_CONNECTION);

    struct sockaddr_in server_addr;
    /* Construct local address structure */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; /* Internet address family */
    server_addr.sin_addr.s_addr = INADDR_ANY; /* Any incoming interface */
    server_addr.sin_len = sizeof(server_addr);
    server_addr.sin_port = htons(SERVER_PORT); /* Local port */

    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(sockfd, (struct sockaddr * )&server_addr, sizeof(server_addr));
    listen(sockfd, SERVER_MAX_CONNECTION);

    int acceptsockfd = -1;
   while (1) {
//       CTcpConn* conn = server.Accept();
       acceptsockfd = accept(sockfd, (struct sockaddr*) NULL, NULL);
       ESP_LOGI(TAG_SRV, "CTcpConn connected...");
       xTaskCreate(tcp_server_handle, "tcp_server_handle", 2048, (void* )acceptsockfd, 6, NULL);
   }
}
