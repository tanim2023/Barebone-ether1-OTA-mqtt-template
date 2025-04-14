#ifndef MY_TASKS_H
#define MY_TASKS_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

// LED PIN Definition
#define BLINK_GPIO GPIO_NUM_2

// Initialize LED GPIO
void led_init(void);

// Blink LED task function
void blink_gpio_task(void *pvParameters);

#endif // MY_TASKS_H