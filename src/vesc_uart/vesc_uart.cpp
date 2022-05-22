#include "vesc_uart.h"

static const char *TAG = "VESC";


static void send_func(unsigned char *data, unsigned int len) {
    int bytesWritten = uart_write_bytes(VESC_PORT, data, len);
    // ESP_LOGI(TAG, "packet written: %d bytes", bytesWritten);
}

static void process_func(unsigned char *data, unsigned int len) {
    // ESP_LOGI(TAG, "packet received: %d bytes", len);
}

VescUart::VescUart() {
    uartConfig = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        UART_SCLK_APB,
    };
}

void VescUart::init() {
    ESP_LOGI(TAG, "Setting up UART for VESC");
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(VESC_PORT, &uartConfig));
    // Set rx/tx pins
    ESP_ERROR_CHECK(uart_set_pin(VESC_PORT, VESC_TX_PIN, VESC_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Install driver
    ESP_ERROR_CHECK(uart_driver_install(VESC_PORT, 1024, 0, 0, NULL, 0));
    ESP_LOGI(TAG, "Done setting up UART for VESC");
}

void VescUart::sendCommand(uint8_t commandBytes[], int length) {
    // ESP_LOGI(TAG, "Sending command to VESC");
    // Make a packet
    PACKET_STATE_t packet {};
    packet_init(&send_func, &process_func, &packet);
    // send the packet with the contents
    packet_send_packet(commandBytes, length, &packet);
    // reset the packet for next use
    packet_reset(&packet);
}


void VescUart::sendDuty(float duty) {
    // make a payload buffer
    uint8_t command[5];
    int index = 0;
    // set the command id byte
    command[index] = COMM_SET_DUTY;
    index++;
    // add the command value
    buffer_append_int32(command, (int32_t)(duty * 100000.0), &index);
    this->sendCommand(command, 5);
}
