#include "driver/i2c.h"
#include "mpu_manager.h"
#include "esp_log.h"
#include "esp_err.h"
#include "config.h"

static const char *TAG = "mpu manager";

MPU6050 mpu;

void setupI2C() {
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
	mpu.setXAccelOffset(-3164);
	mpu.setYAccelOffset(447);
	mpu.setZAccelOffset(848);
	mpu.setXGyroOffset(213);
	mpu.setYGyroOffset(-80);
	mpu.setZGyroOffset(24);
    //mpu.CalibrateAccel(6);
    //mpu.CalibrateGyro(6);
	mpu.setDMPEnabled(true);
}

MPUValues readMPU() {
    uint16_t packetSize = mpu.dmpGetFIFOPacketSize();
    uint8_t mpuIntStatus = mpu.getIntStatus();
    uint16_t fifoCount = mpu.getFIFOCount();
    MPUValues result {};

    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset DMP FIFO to prevent overflows
        mpu.resetFIFO();
    } else {
        // wait for correct available data length, should be a short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
 
        // read a packet from FIFO
        uint8_t fifoBuffer[64];
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        mpu.dmpGetQuaternion(&result.q, fifoBuffer);
        mpu.dmpGetGravity(&result.gravity, &result.q);
        mpu.dmpGetYawPitchRollOnEnd(result.ypr, &result.q, &result.gravity);
    }
    return result;
}