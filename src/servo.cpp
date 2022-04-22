#include "driver/mcpwm.h"
#include "esp_log.h"

static const char *TAG = "servo";

// set duty time
void setServoMicros(int micros) {
    ESP_LOGI(TAG, "Set micros: %d", micros);
    ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, micros));
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

void setServoDeg(int deg) {
    ESP_LOGI(TAG, "Set angle: %d", deg);
    int micros = (deg / 360.0) * (1000) + 1000;
    setServoMicros(micros);
};

void setServoDecimal(float pos) {
    ESP_LOGI(TAG, "Set decimal: %f", pos);
    int micros = pos * 1000 + 1000;
    setServoMicros(micros);
};


int deg = 0;
int degDirection = 1;
void sweepServoAngle(){
    setServoDeg(deg);
    if (degDirection == 1) {
        deg ++;
    } else {
        deg --;
    }
    if (deg == 360) {
        degDirection = 0;
    }
    if (deg == 0) {
        degDirection = 1;
    }
};