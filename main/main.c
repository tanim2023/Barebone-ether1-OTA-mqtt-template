//----------------------------------------------------------------------------------------------
//NAME: Barebone ether1-OTA-mqtt template
//writen by TNM WISH and Cook LTD
//IDF VERSION 5.4.0
//NOTE: PLEASE do not use dynamic memory allocation
//----------------------------------------------------------------------------------------------
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Include our modular headers
#include "common.h"
#include "ethernet.h"
#include "mqtt.h"
#include "ota.h"
#include "my_tasks.h"

static const char *TAG = "main";

// Define any queue in the Global scope
QueueHandle_t mqtt_message_queue = NULL;
QueueHandle_t ota_queue = NULL;

void app_main(void)
{
    ESP_LOGI(TAG, "Starting application...");
    
    // Initialize queues
    mqtt_message_queue = xQueueCreate(10, sizeof(MqttMessage));
    ota_queue = xQueueCreate(10, sizeof(OtaType));
    
    // Check OTA status from previous boot
    ota_check_status();
    
    // Initialize NVS  
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize ethernet
    ethernet_init();
    
    // Create tasks
    xTaskCreate(blink_gpio_task, "blink_gpio_task", 2048, NULL, 5, NULL);
    xTaskCreate(mqtt_task_distribution, "mqtt_task_distribution", 4096, NULL, 5, NULL);
    xTaskCreate(http_ota_task, "http_ota_task", 10240, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "All tasks started");
}