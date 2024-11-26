// Include base C library
#include <stdio.h>

// FreeRTOS library
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP-IDF library
#include <esp_log.h>

// Project library

// Tasks
void taskAnalogMeasure(void *);
void taskStoreData(void *);
void taskReadData(void *);
void taskSendData(void *);

// Main function
void app_main() {
  static char *TAG = "app_main";

  // Create tasks
  if (xTaskCreate(taskAnalogMeasure, "taskAnalogMeasure", 2048, NULL, 1, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Error creating taskAnalogMeasure");
  }
  if (xTaskCreate(taskStoreData, "taskStoreData", 2048, NULL, 1, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Error creating taskStoreData");
  }
  if (xTaskCreate(taskReadData, "taskReadData", 2048, NULL, 1, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Error creating taskReadData");
  }
  if (xTaskCreate(taskSendData, "taskSendData", 2048, NULL, 1, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Error creating taskSendData");
  }
}