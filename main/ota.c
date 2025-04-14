#include "ota.h"
#include "common.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include <string.h> // For strlen and snprintf

static const char *TAG = "ota";

void ota_check_status(void) {
    ESP_LOGI(TAG, "Checking OTA status...");
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            ESP_LOGW(TAG, "Previous OTA marked invalid. Rolling back...");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }
    }
}

void http_ota_task(void *pvParameter) {
    char stat_msg[128]; // Increased buffer size to be safe
    while (1) {
        OtaType ota_msg;
        if (xQueueReceive(ota_queue, &ota_msg, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Received OTA request for URL: %s", ota_msg.url);
            ESP_LOGI(TAG, "Device ID: %s", ota_msg.device);
           
            // Perform OTA update using the data
            if (strlen(ota_msg.url) > 0 && strcmp(DEVICE_NAME, ota_msg.device) == 0) {
                ESP_LOGI(TAG, "valid OTA URL");
                snprintf(stat_msg, sizeof(stat_msg), "{ \"device\": \"%s\", \"status\": \"valid OTA URL\" }", DEVICE_NAME);
                esp_mqtt_client_publish(mqtt_client, MQTT_STATUS_TOPIC, stat_msg, 0, 1, 0);
                
                esp_http_client_config_t http_config = {
                    .url = ota_msg.url,
                    .disable_auto_redirect = false  // Allow redirects if needed
                };

                esp_https_ota_config_t ota_config = {
                    .http_config = &http_config,
                    .bulk_flash_erase = true
                };

                esp_err_t ret = esp_https_ota(&ota_config);
                if (ret == ESP_OK) {
                    snprintf(stat_msg, sizeof(stat_msg), "{ \"device\": \"%s\", \"status\": \"OTA Success\" }", DEVICE_NAME);
                    esp_mqtt_client_publish(mqtt_client, MQTT_STATUS_TOPIC, stat_msg, 0, 1, 0);
                    ESP_LOGI(TAG, "OTA Success! Restarting...");
                    esp_ota_mark_app_valid_cancel_rollback();
                    esp_restart();
                } else {
                    ESP_LOGE(TAG, "OTA Failed: %s", esp_err_to_name(ret));
                    snprintf(stat_msg, sizeof(stat_msg), "{ \"device\": \"%s\", \"status\": \"OTA Failed: %s\" }", DEVICE_NAME, esp_err_to_name(ret));
                    esp_mqtt_client_publish(mqtt_client, MQTT_STATUS_TOPIC, stat_msg, 0, 1, 0);
                }
            } else {
                ESP_LOGE(TAG, "Invalid OTA URL");
                snprintf(stat_msg, sizeof(stat_msg), "{ \"device\": \"%s\", \"status\": \"Invalid URL or device name\" }", DEVICE_NAME);
                esp_mqtt_client_publish(mqtt_client, MQTT_STATUS_TOPIC, stat_msg, 0, 1, 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }            
}