#include "config.h" 
#include "esp_log.h"
#include "esp_err.h"
#include "vesc_uart/vesc_uart.h"
#include "mpu/mpu_manager.h"
#include "pid_controller/pid_controller.h"
#include "MPU6050_6Axis_MotionApps20.h" // ONLY INCLUDE ONCE IN PROJECT
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "input_filters/median_filter.h"

static const char *TAG = "control";

VescUart vesc;
// angle PID keeps the bot upright
// output is in duty cycle
PIDController anglePid = PIDController(2.0, 0.00003, 5000.0, -0.5, 0.5);
// position PID tries to hold place in space
// output is in radians
PIDController positionPid = PIDController(0.001, 0.0, 75.0, -0.4, 0.4);
MPUValues mpuValues;
float outDuty;
int adcReading;
float setpoint = 0.0; // radians
float positionAdjustmentToSetpoint = 0.0;
MedianFilter mFilter = MedianFilter();
int enabled = 0;
int initialTachPosition = 0;

void setupADC() {
    // Check that the channel selected matches the pin configured
    gpio_num_t adcPin;
    ESP_ERROR_CHECK(adc2_pad_get_io_num(BALANCE_REFERENCE_CHANNEL, &adcPin));
    assert(adcPin == BALANCE_REFERENCE_PIN);
    // Set the highest attenuation
    adc2_config_channel_atten(BALANCE_REFERENCE_CHANNEL, ADC_ATTEN_11db);
}

void getSetpoint() {
    // read setpoint
    adc2_get_raw(BALANCE_REFERENCE_CHANNEL, ADC_WIDTH_BIT_12, &adcReading);
    if (mFilter.insert(adcReading)) {
        // Scale adc reading to float -1.0:1.0
        // 12-bit ADC gives values 0 to 2048 (2^12)
        setpoint = ((mFilter.calc() / pow(2, 11)) - 1.0);
        // Scale to -0.1:0.1 for more sensitivity
        setpoint = setpoint;
    }
}

float getPositionAdjustment() {
    // update velocity PID
    // use the position when enabled as setpoint
    float result = positionPid.update(vesc.currentValues.tachometer, initialTachPosition);
    return result;
}

static void sendFunc(unsigned char *data, uint len){
    vesc.sendFunc(data, len);
}

static void processFunc(unsigned char *data, uint len){
    vesc.processFunc(data, len);
}

void controlTask(void* pvParameters) {
    // setup MPU6050
    setupI2C();

    // setup VESC
    vesc = VescUart();
    vesc.init(&sendFunc, &processFunc);

    // setup ADC
    setupADC();

    // setup enable switch
    gpio_pad_select_gpio(ENABLE_PIN);
    gpio_set_direction(ENABLE_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(ENABLE_PIN);

    // for safety, wait for the device to be disabled before starting anything
    while(!gpio_get_level(ENABLE_PIN)) {
        ESP_LOGI(TAG, "Waiting for disable to clear safety");
        vTaskDelay(20/portTICK_PERIOD_MS);
    };

    // get initial tach position for 0 reference
    vesc.sendGetValues();
    vesc.readBytes();
    initialTachPosition = vesc.currentValues.tachometer;

    // setup PID
    anglePid.reset();
    positionPid.reset();

    // begin main loop
    while(true) {
        // Main code goes here
        // get vesc runtime information
        vesc.sendGetValues();
        vesc.readBytes();
        // read angle
        mpuValues = readMPU();
        // check if enabled (currently negated)
        enabled = !gpio_get_level(ENABLE_PIN);
        if (enabled == 1) {
            // get angle setpoint
            getSetpoint();
            // get position adjustment
            positionAdjustmentToSetpoint = getPositionAdjustment();
            // update angle PID to get output
            outDuty = anglePid.update(sin(mpuValues.ypr[1]), sin(/*setpoint + */positionAdjustmentToSetpoint));
            // send command to motor controller
            ESP_LOGI(TAG, "angle %1.3f : position adjust %1.3f : output %1.3f", mpuValues.ypr[1], /*setpoint + */positionAdjustmentToSetpoint, outDuty);
            vesc.sendDuty(-outDuty);
        } else {
            anglePid.reset();
            positionPid.reset();
            initialTachPosition = vesc.currentValues.tachometer;
            vesc.sendDuty(0.0);
            ESP_LOGI(TAG, "disabled");
        }
        //vTaskDelay(0/portTICK_PERIOD_MS);
    }
}