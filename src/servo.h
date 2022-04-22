class Servo {
    gpio_num_t pin;
    int setMicros;
    int minMicros;
    int maxMicros;
    int sweepDirection;

    void setServoMicros(int micros);
public:
    Servo() {};
    Servo(gpio_num_t servoPin);
    void sweepTick(int increment);
    void setAngle(int deg);
    void setDecimal(float pos);
};