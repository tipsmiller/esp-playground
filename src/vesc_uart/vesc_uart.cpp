#include "vesc_uart.h"

static const char *TAG = "VESC";

void VescUart::sendFunc(unsigned char *data, uint len) {
    int bytesWritten = uart_write_bytes(VESC_PORT, data, len);
    //ESP_LOGI(TAG, "packet written: %d bytes", bytesWritten);
}

void VescUart::processFunc(unsigned char *data, uint len) {
    //ESP_LOGI(TAG, "packet received: %d bytes", len);
    // determine packet type and parse
    this->currentValues = decodeValues(data, len); // COMM_GET_VALUES
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

void VescUart::init(void (*sendFunction)(unsigned char *data, unsigned int len), void (*processFunction)(unsigned char *data, unsigned int len)) {
    ESP_LOGI(TAG, "Setting up UART for VESC");
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(VESC_PORT, &uartConfig));
    // Set rx/tx pins
    ESP_ERROR_CHECK(uart_set_pin(VESC_PORT, VESC_TX_PIN, VESC_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Install driver
    ESP_ERROR_CHECK(uart_driver_install(VESC_PORT, 1024, 0, 0, NULL, 0));
    packet_init(sendFunction, processFunction, &packet);
    ESP_LOGI(TAG, "Done setting up UART for VESC");
}

void VescUart::readBytes() {
    uint8_t readBuf[100] {};
    int newBytes = uart_read_bytes(VESC_PORT, &readBuf, 100, 0);
    //ESP_LOGI(TAG, "received %d bytes", newBytes);
    for (int bufIndex = 0; bufIndex < newBytes; bufIndex ++) {
        packet_process_byte(readBuf[bufIndex], &packet);
    }
}

void VescUart::sendCommand(uint8_t commandBytes[], int length) {
    // ESP_LOGI(TAG, "Sending command to VESC");
    // send the packet with the contents
    packet_send_packet(commandBytes, length, &packet);
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

void VescUart::sendGetValues() {
    uint8_t command[1] {COMM_GET_VALUES};
    this->sendCommand(command, 1);
}

mc_values VescUart::decodeValues(unsigned char *data, uint len) {
    mc_values result {};
    int index = 0;
    uint8_t packetId = data[index];
    index++;
    //ESP_LOGI(TAG, "decoding packet with id %d", packetId);
	result.temp_mos = buffer_get_float16(data, 1e1, &index);
	result.temp_motor = buffer_get_float16(data, 1e1, &index);
    result.current_motor = buffer_get_float32(data, 1e2, &index);
    result.current_in = buffer_get_float32(data, 1e2, &index);
    result.id = buffer_get_float32(data, 1e2, &index);
    result.iq = buffer_get_float32(data, 1e2, &index);
    result.duty_now = buffer_get_float16(data, 1e3, &index);
    result.rpm = buffer_get_float32(data, 1e0, &index);
    result.v_in = buffer_get_float16(data, 1e1, &index);
    result.amp_hours = buffer_get_float32(data, 1e4, &index);
    result.amp_hours_charged = buffer_get_float32(data, 1e4, &index);
    result.watt_hours = buffer_get_float32(data, 1e4, &index);
    result.watt_hours_charged = buffer_get_float32(data, 1e4, &index);
    result.tachometer = buffer_get_int32(data, &index);
    result.tachometer_abs = buffer_get_int32(data, &index);
    //result.fault_code = (uint8_t)data[index];  // haven't made a converter from byte to enum
    index++;
    result.position = buffer_get_float32(data, 1e6, &index);
    result.vesc_id = (uint8_t)data[index];
    index++;
    result.temp_mos_1 = buffer_get_float16(data, 1e1, &index);
    result.temp_mos_2 = buffer_get_float16(data, 1e1, &index);
    result.temp_mos_3 = buffer_get_float16(data, 1e1, &index);
    result.vd = buffer_get_float32(data, 1e3, &index);
    result.vq = buffer_get_float32(data, 1e3, &index);
    //uint8_t status = data[index];  // haven't made a converter from byte to enum
    index++;
    //ESP_LOGI(TAG, "got %d bytes out of values packet", index);
    return result;
}
