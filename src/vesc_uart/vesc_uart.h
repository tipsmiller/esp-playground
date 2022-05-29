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
    PACKET_STATE_t packet {};
public:
    mc_values currentValues {};
    VescUart();
    void init(void (*sendFunction)(unsigned char *data, unsigned int len), void (*processFunction)(unsigned char *data, unsigned int len));
    void sendCommand(uint8_t commandBytes[], int length);
    void sendDuty(float duty);
    void readBytes();
    void sendGetValues();
    void sendFunc(unsigned char *data, uint len);
    void processFunc(unsigned char *data, uint len);
    mc_values decodeValues(unsigned char *data, uint len);
};

