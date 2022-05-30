#include "esp_timer.h"

class PIDController {
    private:
    public:
        float kp;
        float ki;
        float kd;
        int minimumCycleMs = 1;
        int64_t lastTime; // microseconds
        float lastError;
        float lastInput;
        float lastOutput;
        float outMin;
        float outMax;
        float errorIntegral;
        PIDController(float kp, float ki, float kd, float outMin, float outMax);
        float update(float input, float setpoint);
        void setCoefficients(float kp, float ki, float kd);
        void setLimits(float outMin, float outMax);
        void setMinimumCycleTime(int minimumCycleMs);
        void reset();
};