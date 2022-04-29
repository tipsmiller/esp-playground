/* adapted from vedderb/bldc */

#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>
#include <stdbool.h>

// Settings
#ifndef PACKET_MAX_PL_LEN
#define PACKET_MAX_PL_LEN		512
#endif

#define PACKET_BUFFER_LEN		(PACKET_MAX_PL_LEN + 8)

// Types
typedef struct {
	void(*send_func)(unsigned char *data, unsigned int len);
	void(*process_func)(unsigned char *data, unsigned int len);
	unsigned int rx_read_ptr;
	unsigned int rx_write_ptr;
	int bytes_left;
	unsigned char rx_buffer[PACKET_BUFFER_LEN];
	unsigned char tx_buffer[PACKET_BUFFER_LEN];
} PACKET_STATE_t;

// Functions
void packet_init(void (*s_func)(unsigned char *data, unsigned int len),
		void (*p_func)(unsigned char *data, unsigned int len), PACKET_STATE_t *state);
void packet_reset(PACKET_STATE_t *state);
void packet_process_byte(uint8_t rx_data, PACKET_STATE_t *state);
void packet_send_packet(unsigned char *data, unsigned int len, PACKET_STATE_t *state);

#endif /* PACKET_H_ */
