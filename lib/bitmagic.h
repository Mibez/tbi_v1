/**
* @file     bitmagic.h
* @brief    Header file for helpers to stuff bits into a buffer and out of it,
            disrespecting byte boundaries, and various other bit "magic"
*/

#ifndef __TBI_BITMAGIC_H
#define __TBI_BITMAGIC_H

#include <stdint.h>

void test_bit_pack(void);



void bit_pack(uint8_t* result_buf, uint32_t* in_buf, int target_bits, int *bit_ptr);
void bit_unpack(uint8_t* in_buf, uint32_t* result_buf, int target_bits, int *bit_ptr);

uint8_t min_bits_32(uint32_t val, bool is_signed);
uint8_t min_bits_16(uint16_t val, bool is_signed);
uint8_t min_bits_8(uint8_t val, bool is_signed);


#endif /* __TBI_BITMAGIC_H */