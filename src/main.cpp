#include "config.h" 
#include "heartbeat/heartbeat.h"
#include "esp_log.h"
#include "esp_err.h"
#include "vesc_uart/vesc_uart.h"
#include "mpu/mpu_manager.h"

static const char *TAG = "main";
VescUart vesc;

void loop() {
    // Main code goes here
    readMPU();
    //vesc.sendCommand();
    vTaskDelay(10/portTICK_PERIOD_MS);
}


extern "C" void app_main() {
    // Setup
    ESP_LOGI(TAG, "Program beginning");

    // start the heartbeat
    beginHeartbeat();

    // setup MPU6050
    setupI2C();

    // setup VESC
    vesc = VescUart();
    vesc.init();

    // Begin main loop
    while(true) {
        loop();
    }

    // Cleanup
}