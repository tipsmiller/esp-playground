#include "driver/uart.h"
#include "driver/gpio.h"
#include "datatypes.h"
#include "buffer.h"
#include "crc.h"
#include "esp_log.h"
#include "packet.h"
#include "config.h"

class VescUart {
private:
    uart_port_t serialPort;
    gpio_num_t rxPin;
    gpio_num_t txPin;
    uart_config_t uartConfig;
public:
    VescUart();
    void init();
    void sendCommand();
};

