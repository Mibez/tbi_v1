/**
* @file     crc16.h
* @brief    CRC16-CCITT implementation
*/

#include "crc16.h"

#define POLYNOMIAL_CRC16_CCITT 0x1021

/** @brief Get CRC16-CCITT initialization value when starting new computation */
uint16_t crc16_begin(void) {
    return 0xffff;
}

/**
 * @brief Compute next CRC16 value
 * 
 * @param[in] crc       CRC for previous byte, or initialization value from @ref crc16_begin()
 * @param[in] new_val   Next byte
 * 
 * @return CRC16 value of current and all previous bytes
 */
uint16_t crc16(uint16_t crc, uint8_t new_val) 
{
	int i;

	for (i = 0; i < 8; i++) {

		if (((crc & 0x8000) >> 8) ^ (new_val & 0x80)){
			crc = (crc << 1)  ^ POLYNOMIAL_CRC16_CCITT;
		}else {
			crc = (crc << 1);
		}

		new_val <<= 1;
	}
  
	return crc;
}
