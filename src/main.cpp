#include "stdio.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "config.h" 
#include "heartbeat.h"
#include "esp_log.h"
#include "esp_err.h"
#include "servo.h"
#include "http_server.h"
#include "spiffs.h"

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

extern "C" void app_main() {
    // Setup
    ESP_LOGI(TAG, "Program beginning");
    // init storage
    static const char* base_path = "/data";
    ESP_ERROR_CHECK(mountSpiffs(base_path));
    // start networking and server
    ESP_ERROR_CHECK(startWebserver(base_path));
    // start the heartbeat
    beginHeartbeat();
    // setup the servo
    sweeper = Servo(GPIO_NUM_13);


    // Begin main loop
    while(true) {
        loop();
    }

    // Cleanup
}