/* adapted from vedderb/bldc */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

/*
 * Functions
 */
unsigned short crc16(unsigned char *buf, unsigned int len);
uint32_t crc32(uint32_t *buf, uint32_t len);
void crc32_reset(void);

#endif /* CRC_H_ */
