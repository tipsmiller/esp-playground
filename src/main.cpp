#include "stdio.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "config.h" 
#include "heartbeat.h"
#include "esp_log.h"
#include "servo.h"

static const char *TAG = "main";

void loop() {
    // Main code goes here
    sweepServoAngle();
    vTaskDelay(10/portTICK_PERIOD_MS);
}

/*void printStats() {
    char stats [256];
    vTaskGetRunTimeStats(stats);
    printf(stats);
}*/

void beginHeartbeat();

extern "C" void app_main() {
    ESP_LOGI(TAG, "Program beginning");
    beginHeartbeat();
    initServo(13);
    while(true) {
        loop();
    }
}