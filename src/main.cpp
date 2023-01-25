#include "config.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "heartbeat/heartbeat.h"
#include "esp_log.h"
#include "esp_err.h"
#include "http_server/spiffs.h"
#include "http_server/wifi_client.h"
#include "http_server/http_server.h"
#include "control_loop.h"

static const char *TAG = "main";
static const char* baseFilePath = "/data";

QueueHandle_t wsMessageQueue;


extern "C" void app_main() {
    // Setup
    ESP_LOGI(TAG, "Program beginning");

    // start the heartbeat
    beginHeartbeat();

    // setup message queue http -> control
    wsMessageQueue = xQueueCreate(10, sizeof(int));

    // startup control task
    setControlQueue(wsMessageQueue);
    TaskHandle_t controlTaskHandle;
    xTaskCreate(controlTask, "control loop", 10000, (void*)NULL, configMAX_PRIORITIES-1, &controlTaskHandle);

    // start file system
    // ESP_ERROR_CHECK(mountSpiffs(baseFilePath));
    // start webserver
    // ESP_ERROR_CHECK(startWebserver(baseFilePath, wsMessageQueue));

    // Cleanup
}


/*
//Code below runs the calibration for the MPU6050
#include "mpu/mpu_calibration.h"
#include "MPU6050_6Axis_MotionApps20.h" // ONLY INCLUDE ONCE IN PROJECT

extern "C" void app_main() {
    CalibrateMPU();
}
*/