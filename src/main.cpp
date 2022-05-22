#include "config.h" 
#include "heartbeat/heartbeat.h"
#include "esp_log.h"
#include "esp_err.h"
#include "vesc_uart/vesc_uart.h"
#include "mpu/mpu_manager.h"
#include "pid_controller/pid_controller.h"
#include "MPU6050_6Axis_MotionApps20.h" // ONLY INCLUDE ONCE IN PROJECT
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "input_filters/median_filter.h"

static const char *TAG = "main";
VescUart vesc;
PIDController pid = PIDController(1.0, 0.0, 0.0, -1.0, 1.0);
MPUValues mpuValues;
float outDuty;
int adcReading;
float setpoint = 0.0;
MedianFilter mFilter = MedianFilter();
int enabled = 0;

void loop() {
    // Main code goes here
    // read angle
    mpuValues = readMPU();
    // read setpoint
    adc2_get_raw(BALANCE_REFERENCE_CHANNEL, ADC_WIDTH_BIT_12, &adcReading);
    if (mFilter.insert(adcReading)) {
        // Scale adc reading to float -1.0:1.0
        // 12-bit ADC gives values 0 to 2048 (2^12)
        setpoint = (mFilter.calc() / pow(2, 11)) - 1.0;
    }
    // check if enabled
    enabled = gpio_get_level(ENABLE_PIN);
    if (enabled == 1) {
        // update PID to get output
        outDuty = pid.update(mpuValues.ypr[1], setpoint);
        // send command to motor controller
        ESP_LOGI(TAG, "output %1.3f", outDuty);
        vesc.sendDuty(outDuty);
    } else {
        pid.reset();
        ESP_LOGI(TAG, "disabled");
        vesc.sendDuty(0.0);
    }
    vTaskDelay(0/portTICK_PERIOD_MS);
}

void setupADC() {
    // Check that the channel selected matches the pin configured
    gpio_num_t adcPin;
    ESP_ERROR_CHECK(adc2_pad_get_io_num(BALANCE_REFERENCE_CHANNEL, &adcPin));
    assert(adcPin == BALANCE_REFERENCE_PIN);
    // Set the highest attenuation
    adc2_config_channel_atten(BALANCE_REFERENCE_CHANNEL, ADC_ATTEN_11db);
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

    // setup ADC
    setupADC();

    // setup enable switch
    gpio_pad_select_gpio(ENABLE_PIN);
    gpio_set_direction(ENABLE_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(ENABLE_PIN);

    // setup PID
    pid.reset();

    // Begin main loop
    while(true) {
        loop();
    }

    // Cleanup
}