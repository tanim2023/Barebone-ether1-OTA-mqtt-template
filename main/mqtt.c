#include "mqtt.h"
#include "common.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "mqtt_client.h"
#include <string.h> // For strncpy and strcmp

static const char *TAG = "mqtt";
esp_mqtt_client_handle_t mqtt_client = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch(event->event_id) {
        case MQTT_EVENT_CONNECTED:
        char stat_msg[150];
            ESP_LOGI(TAG, "MQTT connected");
            //subscribe any topic that is declared in common.h please do not directly wite topic name here
            //-------------------------SUBSCRIBE TOPIC START---------------------------------------------
            esp_mqtt_client_subscribe(mqtt_client, MQTT_UPDATE_TOPIC, 1);

            //--------------------------SUBSCRIBE TOPIC END----------------------------------------------
            snprintf(stat_msg, sizeof(stat_msg), "{ \"device\": \"%s\", \"ip\": \"%s\", \"mac\": \"%02x:%02x:%02x:%02x:%02x:%02x\" , \"status\": \"online\"}", DEVICE_NAME, 
                                                    actual_ip, assigned_mac[0], assigned_mac[1], assigned_mac[2], assigned_mac[3], assigned_mac[4], assigned_mac[5]);
            esp_mqtt_client_publish(mqtt_client, MQTT_STATUS_TOPIC, stat_msg, 0, 1, 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT data received on topic %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "(mqtt_event_handler) Received JSON: %.*s", event->data_len, event->data);
            
            MqttMessage message;
            strncpy(message.topic, event->topic, event->topic_len);
            message.topic[event->topic_len] = '\0';
            strncpy(message.data, event->data, event->data_len);
            message.data[event->data_len] = '\0';
            
            if (xQueueSend(mqtt_message_queue, &message, pdMS_TO_TICKS(100)) == pdTRUE) {
                ESP_LOGI(TAG, "Message sent to queue successfully");
            } else {
                ESP_LOGE(TAG, "Failed to send message to queue");
            }
            break;

        default:
            break;
    }
}

void mqtt_app_start(void)
{
    char MQTT_LWT_MESSAGE[150];
    snprintf(MQTT_LWT_MESSAGE, sizeof(MQTT_LWT_MESSAGE), "{ \"device\": \"%s\", \"ip\": \"%s\", \"mac\": \"%02x:%02x:%02x:%02x:%02x:%02x\" , \"status\": \"online\"}", DEVICE_NAME, 
                                                            actual_ip, assigned_mac[0], assigned_mac[1], assigned_mac[2], assigned_mac[3], assigned_mac[4], assigned_mac[5]);
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .session.last_will.topic = MQTT_STATUS_TOPIC,
        .session.last_will.msg = MQTT_LWT_MESSAGE,
        .session.keepalive = 60,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void mqtt_task_distribution(void *pvParameter) {
    while (1) {
        MqttMessage message;
        if (xQueueReceive(mqtt_message_queue, &message, portMAX_DELAY)) {
            ESP_LOGI(TAG, "(mqtt_task_distribution) Received MQTT message on topic %s: %s", message.topic, message.data);
            if (strcmp(message.topic, MQTT_UPDATE_TOPIC) == 0) {
                // Create a message with fixed size
                OtaType ota_msg = {0}; // Zero-initialize the structure
               
                // Parse JSON and fill the structure
                cJSON *root = cJSON_Parse(message.data);
                if (root) {
                    cJSON *url = cJSON_GetObjectItem(root, "ota_url");
                    if (cJSON_IsString(url) && url->valuestring) {
                        strncpy(ota_msg.url, url->valuestring, sizeof(ota_msg.url) - 1);
                    }
                   
                    cJSON *device = cJSON_GetObjectItem(root, "device");
                    if (cJSON_IsString(device) && device->valuestring) {
                        strncpy(ota_msg.device, device->valuestring, sizeof(ota_msg.device) - 1);
                    }
                   
                    cJSON_Delete(root);
                   
                    // Send the fixed-size structure
                    if (xQueueSend(ota_queue, &ota_msg, pdMS_TO_TICKS(100)) == pdTRUE) {
                        ESP_LOGI(TAG, "Message sent to ota_queue successfully");
                    } else {
                        ESP_LOGE(TAG, "Failed to send message to ota_queue");
                    }
                } else {
                    ESP_LOGE(TAG, "Failed to parse JSON message");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}