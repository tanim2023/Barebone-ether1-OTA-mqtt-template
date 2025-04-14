#ifndef MQTT_H
#define MQTT_H

// Initialize and start the MQTT client
void mqtt_app_start(void);

// MQTT task that processes messages from queue
void mqtt_task_distribution(void *pvParameter);

#endif // MQTT_H