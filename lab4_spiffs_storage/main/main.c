#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#define SPIFFS_BASE_PATH       "/spiffs"
#define SPIFFS_PARTITION_NAME  "storage"
#define LOG_FILE_PATH          SPIFFS_BASE_PATH "/log.txt"

static const char *TAG = "SPIFFS_LAB";
static const char *EXPECTED_TEXT =
    "Sensor Log Entry #104: Temp=24.5C, Humidity=60%";

void app_main(void)
{
    esp_vfs_spiffs_conf_t spiffs_config = {
        .base_path = SPIFFS_BASE_PATH,
        .partition_label = SPIFFS_PARTITION_NAME,
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    ESP_LOGI(TAG, "Mounting SPIFFS partition '%s' at %s",
             SPIFFS_PARTITION_NAME,
             SPIFFS_BASE_PATH);

    esp_err_t result = esp_vfs_spiffs_register(&spiffs_config);
    if (result != ESP_OK) {
        if (result == ESP_FAIL) {
            ESP_LOGE(TAG, "SPIFFS mount or format failed");
        } else if (result == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Partition '%s' was not found",
                     SPIFFS_PARTITION_NAME);
        } else {
            ESP_LOGE(TAG, "SPIFFS initialization failed: %s",
                     esp_err_to_name(result));
        }
        return;
    }

    size_t total_bytes = 0;
    size_t used_bytes = 0;
    result = esp_spiffs_info(SPIFFS_PARTITION_NAME,
                             &total_bytes,
                             &used_bytes);
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS size: total=%u bytes, used=%u bytes",
                 (unsigned int)total_bytes,
                 (unsigned int)used_bytes);
    } else {
        ESP_LOGW(TAG, "Unable to read SPIFFS size: %s",
                 esp_err_to_name(result));
    }

    FILE *write_file = fopen(LOG_FILE_PATH, "w");
    if (write_file == NULL) {
        ESP_LOGE(TAG, "Unable to open %s for writing", LOG_FILE_PATH);
        esp_vfs_spiffs_unregister(SPIFFS_PARTITION_NAME);
        return;
    }

    int characters_written = fprintf(write_file, "%s\n", EXPECTED_TEXT);
    int close_result = fclose(write_file);
    if (characters_written < 0 || close_result != 0) {
        ESP_LOGE(TAG, "Writing or flushing %s failed", LOG_FILE_PATH);
        esp_vfs_spiffs_unregister(SPIFFS_PARTITION_NAME);
        return;
    }
    ESP_LOGI(TAG, "Wrote %d characters to %s",
             characters_written,
             LOG_FILE_PATH);

    FILE *read_file = fopen(LOG_FILE_PATH, "r");
    if (read_file == NULL) {
        ESP_LOGE(TAG, "Unable to open %s for reading", LOG_FILE_PATH);
        esp_vfs_spiffs_unregister(SPIFFS_PARTITION_NAME);
        return;
    }

    char readback[128] = {0};
    char *read_result = fgets(readback, sizeof(readback), read_file);
    fclose(read_file);

    if (read_result == NULL) {
        ESP_LOGE(TAG, "Reading %s failed", LOG_FILE_PATH);
        esp_vfs_spiffs_unregister(SPIFFS_PARTITION_NAME);
        return;
    }

    // Remove CR/LF before comparing the text.
    readback[strcspn(readback, "\r\n")] = '\0';
    ESP_LOGI(TAG, "Read back: %s", readback);

    if (strcmp(readback, EXPECTED_TEXT) == 0) {
        ESP_LOGI(TAG, "PASS: SPIFFS readback matches the written data");
    } else {
        ESP_LOGE(TAG, "FAIL: SPIFFS readback does not match");
    }

    esp_vfs_spiffs_unregister(SPIFFS_PARTITION_NAME);
    ESP_LOGI(TAG, "SPIFFS unmounted cleanly");
}
