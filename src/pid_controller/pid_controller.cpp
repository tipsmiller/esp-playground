#include "pid_controller.h"

PIDController::PIDController(float kp, float ki, float kd, float outMin, float outMax) {
    this->setCoefficients(kp, ki, kd);
    this->setLimits(outMin, outMax);
    this->reset();
}

void PIDController::setCoefficients(float kp, float ki, float kd) {
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
}

void PIDController::setLimits(float outMin, float outMax) {
    this->outMin = outMin;
    this->outMax = outMax;
}

void PIDController::setMinimumCycleTime(int minimumCycleMs) {
    this->minimumCycleMs = minimumCycleMs;
}

float PIDController::update(float input, float setpoint) {
    // get the time
    int64_t newTime = esp_timer_get_time();
    int64_t timeDelta = newTime - this->lastTime;
    float output = 0.0;
    // see if it's been long enough to produce a new value
    if(timeDelta / 1000 > this->minimumCycleMs) {
        // Run the calc
        float error = setpoint - input;
        // proportional
        output = this->kp * error;
        // if this is NOT the first run, calculate the derivative and integral
        if(this->lastTime != 0) {
            // integral
            // Uses a right-side rectangle integration rule, no interpolation
            this->errorIntegral += this->ki * timeDelta * error;
            // clamp the integral to prevent wind-up and crazy overshoots
            if(this->errorIntegral > this->outMax) {
                this->errorIntegral = this->outMax;
            }
            if(this->errorIntegral < this->outMin) {
                this->errorIntegral = this->outMin;
            }
            output += this->errorIntegral;

            //derivative
            output += this->kd * ((error - this->lastError) / timeDelta);
        }
        // clamp the output
        if(output > this->outMax) {
            output = this->outMax;
        }
        if(output < this->outMin) {
            output = this->outMin;
        }
        // save the results
        this->lastError = error;
        this->lastInput = input;
        this->lastOutput = output;
        this->lastTime = newTime;
    } else {
        return this->lastOutput;
    }
    return output;
}

void PIDController::reset() {
    this->lastInput = 0;
    this->lastOutput = 0;
    this->lastTime = 0;
    this->errorIntegral = 0;
}
