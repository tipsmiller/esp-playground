#include "config.h" 
#include "heartbeat.h"
#include "esp_log.h"
#include "esp_err.h"
#include "servo.h"
#include "http_server.h"
#include "spiffs.h"
#include "MPU6050.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <driver/i2c.h>
#include "VescUart/VescUart.h"

static const char *TAG = "main";
Servo sweeper;
MPU6050 mpu;
VescUart vesc;

/*void printStats() {
    char stats [256];
    vTaskGetRunTimeStats(stats);
    printf(stats);
}*/

/*void setupI2C() {
	i2c_config_t conf {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_CLK,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = 400000,},
        .clk_flags = 0,
    };
    ESP_LOGI(TAG, "Configuring I2C");
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_LOGI(TAG, "Installing I2C driver");
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    mpu = MPU6050();
    ESP_LOGI(TAG, "Initializing MPU6050");
    mpu.initialize();
    ESP_LOGI(TAG, "Verifying MPU6050 connection");
    mpu.testConnection();
    ESP_LOGI(TAG, "Setting up MPU6050 DMP");
	mpu.dmpInitialize();
	// This need to be setup individually
	mpu.setXGyroOffset(220);
	mpu.setYGyroOffset(76);
	mpu.setZGyroOffset(-85);
	mpu.setZAccelOffset(1788);
	mpu.setDMPEnabled(true);
}

void readMPU() {
    uint16_t packetSize = mpu.dmpGetFIFOPacketSize();
    uint8_t mpuIntStatus = mpu.getIntStatus();
    // get current FIFO count
     uint16_t fifoCount = mpu.getFIFOCount();

    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();

    // otherwise, check for DMP data ready interrupt frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        uint8_t fifoBuffer[64];
        Quaternion q;           // [w, x, y, z]         quaternion container
        VectorFloat gravity;    // [x, y, z]            gravity vector
        float ypr[3];           // [yaw, pitch, roll]   ypr container
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        printf("YAW: %3.1f, ", ypr[0] * 180/M_PI);
        printf("PITCH: %3.1f, ", ypr[1] * 180/M_PI);
        printf("ROLL: %3.1f \n", ypr[2] * 180/M_PI);
    }
}*/

void loop() {
    // Main code goes here
    //sweeper.sweepTick(1);
    //readMPU();
    vesc.sendCommand();
    vTaskDelay(1000/portTICK_PERIOD_MS);
}


extern "C" void app_main() {
    // Setup
    ESP_LOGI(TAG, "Program beginning");

    // begin i2c
    //setupI2C();

    // init storage
    //static const char* base_path = "/data";
    //ESP_ERROR_CHECK(mountSpiffs(base_path));

    // start networking and server
    //ESP_ERROR_CHECK(startWebserver(base_path));

    // start the heartbeat
    beginHeartbeat();

    // setup the servo
    //sweeper = Servo(GPIO_NUM_13);

    // setup VESC
    vesc.init();

    // Begin main loop
    while(true) {
        loop();
    }

    // Cleanup
}