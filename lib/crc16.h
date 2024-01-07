/**
* @file     crc16.h
* @brief    Header file for CRC16-CCITT implementation
*/

#ifndef __TBI_CRC16_H
#define __TBI_CRC16_H

#include <stdint.h>

uint16_t crc16_begin(void);
uint16_t crc16(uint16_t crc, uint8_t new_val);

#endif /* __TBI_CRC16_H */
