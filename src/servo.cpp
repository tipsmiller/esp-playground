#include "driver/mcpwm.h"
#include "esp_log.h"
#include "servo.h"

static const char *TAG = "servo";

Servo::Servo(gpio_num_t servoPin) {
    pin = servoPin;
    minMicros = 500;
    maxMicros = 2500;
    setMicros = minMicros;
    sweepDirection = 1;

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, pin);

    mcpwm_config_t pwm_config = {
        .frequency = 50,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
};

// set duty time
void Servo::setServoMicros(int micros) {
    setMicros = micros;
    ESP_LOGI(TAG, "Set micros: %d", setMicros);
    ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, setMicros));
}

void initServo(int pin) {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, pin);

    mcpwm_config_t pwm_config = {
        .frequency = 50,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
};

void Servo::setAngle(int deg) {
    ESP_LOGI(TAG, "Set angle: %d", deg);
    int m = (deg / 360.0) * (1000) + 1000;
    setServoMicros(m);
};

void Servo::setDecimal(float pos) {
    ESP_LOGI(TAG, "Set decimal: %f", pos);
    int m = pos * 1000 + 1000;
    setServoMicros(m);
};

void Servo::sweepTick(int increment){
    int micros = setMicros;
    if (sweepDirection == 1) {
        micros += increment;
    } else {
        micros -= increment;
    }
    if (micros == maxMicros) {
        sweepDirection = 0;
    }
    if (micros == minMicros) {
        sweepDirection = 1;
    }
    setServoMicros(micros);
};