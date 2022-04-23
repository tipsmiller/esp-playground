class Servo {
    gpio_num_t pin;
    int set_micros;
    int min_micros;
    int max_micros;
    int sweep_direction;

    void setServoMicros(int micros);
public:
    Servo() {};
    Servo(gpio_num_t servo_pin);
    void sweepTick(int increment);
    void setAngle(int deg);
    void setDecimal(float pos);
};