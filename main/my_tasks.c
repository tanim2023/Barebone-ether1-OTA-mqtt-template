#include "my_tasks.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "led";

void led_init(void) {
    ESP_LOGI(TAG, "Initializing LED on GPIO %d", BLINK_GPIO);
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void blink_gpio_task(void *pvParameters) {
    led_init();
    while (1) {
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(8));
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(8));
    }
}