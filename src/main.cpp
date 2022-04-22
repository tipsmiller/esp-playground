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
Servo sweeper;

void loop() {
    // Main code goes here
    sweeper.sweepTick(1);
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
    sweeper = Servo(GPIO_NUM_13);
    while(true) {
        loop();
    }
}