#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define BUTTON_PIN          GPIO_NUM_4
#define EVENT_QUEUE_LENGTH  10
#define DEBOUNCE_TIME_MS    150

static const char *TAG = "BUTTON_INTERRUPT_LAB";
static QueueHandle_t button_event_queue;

// ISR must stay short: it only copies the GPIO number into a FreeRTOS queue.
static void IRAM_ATTR button_isr_handler(void *argument)
{
    uint32_t gpio_number = (uint32_t)(uintptr_t)argument;
    BaseType_t higher_priority_task_woken = pdFALSE;

    xQueueSendFromISR(button_event_queue,
                      &gpio_number,
                      &higher_priority_task_woken);

    if (higher_priority_task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void button_worker_task(void *argument)
{
    uint32_t gpio_number;
    uint32_t press_count = 0;
    TickType_t last_accepted_press =
        xTaskGetTickCount() - pdMS_TO_TICKS(DEBOUNCE_TIME_MS);

    while (true) {
        if (xQueueReceive(button_event_queue,
                          &gpio_number,
                          portMAX_DELAY) == pdTRUE) {
            TickType_t now = xTaskGetTickCount();

            // Ignore extra falling edges caused by mechanical switch bounce.
            if ((now - last_accepted_press) <
                pdMS_TO_TICKS(DEBOUNCE_TIME_MS)) {
                continue;
            }

            // Wait briefly, then confirm that the active-low button is held.
            vTaskDelay(pdMS_TO_TICKS(20));
            if (gpio_get_level((gpio_num_t)gpio_number) == 0) {
                press_count++;
                last_accepted_press = xTaskGetTickCount();
                ESP_LOGI(TAG,
                         "Button pressed: count=%" PRIu32 ", GPIO=%" PRIu32,
                         press_count,
                         gpio_number);
            }
        }
    }
}

void app_main(void)
{
    button_event_queue = xQueueCreate(EVENT_QUEUE_LENGTH,
                                      sizeof(uint32_t));
    if (button_event_queue == NULL) {
        ESP_LOGE(TAG, "Unable to create the button event queue");
        return;
    }

    BaseType_t task_created = xTaskCreate(button_worker_task,
                                          "button_worker",
                                          2048,
                                          NULL,
                                          10,
                                          NULL);
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Unable to create the button worker task");
        vQueueDelete(button_event_queue);
        return;
    }

    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&button_config));

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(gpio_isr_handler_add(
        BUTTON_PIN,
        button_isr_handler,
        (void *)(uintptr_t)BUTTON_PIN));

    ESP_LOGI(TAG, "Ready. Press the button connected to GPIO 4.");
}
