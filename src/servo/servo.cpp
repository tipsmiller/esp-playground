#include "servo.h"

static const char *TAG = "servo";

Servo::Servo(gpio_num_t servo_pin) {
    pin = servo_pin;
    min_micros = 500;
    max_micros = 2500;
    set_micros = min_micros;
    sweep_direction = 1;

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
    set_micros = micros;
    //ESP_LOGI(TAG, "Set micros: %d", set_micros);
    ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, set_micros));
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
    int micros = set_micros;
    if (sweep_direction == 1) {
        micros += increment;
    } else {
        micros -= increment;
    }
    if (micros == max_micros) {
        sweep_direction = 0;
    }
    if (micros == min_micros) {
        sweep_direction = 1;
    }
    setServoMicros(micros);
};