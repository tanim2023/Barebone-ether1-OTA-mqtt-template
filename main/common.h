#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "mqtt_client.h"  // Correct MQTT include

// Device and MQTT configuration
#define DEVICE_NAME             "ota_base"
#define MQTT_BROKER_URI         "mqtt://192.168.0.50"
#define MQTT_UPDATE_TOPIC       "device_name/update"
#define MQTT_STATUS_TOPIC       "device_name/status"

//IP configuration for ethernet
#define DEVICE_IP      "192.168.0.90"
#define DEVICE_SUBNET  "255.255.255.0"
#define DEVICE_GETWAY  "192.168.0.1"


//Actuall IP Holder
extern char actual_ip[16];
extern uint8_t assigned_mac[6];

// OTA message structure
typedef struct {
    char device[64];  // Adjust size as needed
    char url[256];    // Adjust size as needed
} OtaType;

// MQTT message structure 
typedef struct {
    char topic[64];   // Adjust size as needed
    char data[256];   // Adjust size as needed
} MqttMessage;

// Global handles and queues (declared as extern)
extern esp_mqtt_client_handle_t mqtt_client;
extern QueueHandle_t mqtt_message_queue;
extern QueueHandle_t ota_queue;

#endif // COMMON_H