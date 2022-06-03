#include "esp_timer.h"

class PIDController {
    private:
    public:
        float kp;
        float ki;
        float kde;
        float kdi;
        int minimumCycleMs = 1;
        int64_t lastTime; // microseconds
        float lastError;
        float lastInput;
        float lastDInput;
        float lastOutput;
        float outMin;
        float outMax;
        float errorIntegral;
        float pdiSmoothing = 0; // set to not 0 to enable pdi smoothing
        PIDController(float kp, float ki, float kde, float kdi, float outMin, float outMax);
        float update(float input, float setpoint);
        void setCoefficients(float kp, float ki, float kde, float kdi);
        void setLimits(float outMin, float outMax);
        void setMinimumCycleTime(int minimumCycleMs);
        void reset();
        void setPdiSmoothing(float smoothing);
};