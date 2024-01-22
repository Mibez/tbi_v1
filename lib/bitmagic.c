/**
* @file     bitmagic.c
* @brief    Implementation for helpers to stuff bits into a buffer and out of it,
            disrespecting byte boundaries, and various other bit "magic"
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "bitmagic.h"

/**
 * @brief Pack bits to a buffer
 * 
 * @param[out] result_buf   Buffer to store results to, starting at bit index @ref *bit_ptr
 * @param[in] in_buf        Input value to read bits from
 * @param[in] target_bits   Number of bits that should be used to represent this number
 * @param[in,out] bit_ptr   Bit index of last operation, and output of the same thing (how many bits stored in @ref result_buf)
 * 
*/
void bit_pack(uint8_t* result_buf, uint32_t* in_buf, int target_bits, int *bit_ptr)
{
    int remain, pos, byte_pos;
    uint8_t *res_ptr;

    if(!result_buf || !in_buf || !bit_ptr || *bit_ptr < 0)
        return;
    
    pos = *bit_ptr;
    res_ptr = result_buf;

    uint32_t val = *in_buf;  /* Unmarshal value from buf */
    uint32_t mask = 0;
    uint8_t temp = 0;

    remain = target_bits;
    while(remain > 0) {

        /* If bit pointer is larger than byte, increment out pointer, and make pos from start of byte */
        if(pos >= 8) {
            res_ptr += pos / 8;
            pos %= 8;
            if(pos == 0)
                *res_ptr = 0U;
        }
        
        /* Create mask to discard used bits */
        mask = (((uint32_t)0 - 1) >> (32 - remain));        

        /* If less bits remain than needed to fill byte, Shift value left to fill missing bits */
        if(remain < (8 - pos)) {                            
            temp = (val & mask) << (8 - pos) - remain;          
        } else {
            /* Otherwise take only the bits from value that fit the current out byte */
            temp = (val & mask) >> (remain - (8 - pos));        
        }

        *res_ptr = *res_ptr | temp;     /* Append to out byte */
        remain -= (8 - pos);            /* Decrement remaining bytes with amount we just used */
        pos += (8 - pos);               /* Increment bit position as index from start of byte */

    }

    *bit_ptr += target_bits;

}

/**
 * @brief Unpack bits from a buffer
 * 
 * @param[in] in_buf        Buffer to read bits from, starting at bit index @ref *bit_ptr
 * @param[out] result_buf   Pointer to value to store results in
 * @param[in] target_bits   Number of bits that was used to represent this number
 * @param[in,out] bit_ptr   Bit index of last operation, and output of the same thing (how many bits stored in @ref result_buf)
 * 
*/
void bit_unpack(uint8_t* in_buf, uint32_t* result_buf, int target_bits, int *bit_ptr)
{
    int remain, pos, byte_pos;
    uint32_t val = 0;
    uint8_t current = 0;
    uint8_t mask = 0;

    if(!result_buf || !in_buf || !bit_ptr || *bit_ptr < 0)
        return;

    pos = *bit_ptr;
    remain = target_bits;

    while(remain > 0) {
        byte_pos = pos % 8;
        if(byte_pos == 0)
            byte_pos = 8;
        

        current = *(in_buf + ((pos - 1) / 8));
        mask = ((uint8_t)0 - 1);

        /* Create mask to hide leftmost bits if there's more than we need */
        if(remain < byte_pos)
            mask = mask >> (byte_pos - remain); 

        /* Take only interesting bits and and align them to right */
        current &= mask;
        current = current >> 8 - byte_pos; 
        
        /* Insert into final value */
        val = val | ((uint32_t)current << (target_bits - remain));

        /* Decrement by how many bits we inserted */
        remain -= byte_pos;   
        pos -= byte_pos;
    }

    *result_buf = val;
    *bit_ptr -= target_bits;

}


/**
 * @brief Get minimum number of bits needed to represent the number without sign
 * 
 * @param[in] val       Value as unsigned integer
 * @param[in] signed    Whether or not to consider first bit as sign (discarded)
 *
 * @return minimum number of bits, i.e. index of highest 1
*/
uint8_t min_bits_32(uint32_t val, bool is_signed)
{
    int i, len = 0;

    if((int32_t)val < 0) {
        val = ~val + 1;
    }
    for(i = 31; i >= 0; i--) {
        if((val & (1 << i)) != 0)
            break;
    }
    len += i;
    return len + 1;
}

/**
 * @brief Get minimum number of bits needed to represent the number without sign
 * 
 * @param[in] val   Value as unsigned integer
 * @param[in] signed    Whether or not to consider first bit as sign (discarded)
 *
 * @return minimum number of bits, i.e. index of highest 1
*/
uint8_t min_bits_16(uint16_t val, bool is_signed)
{
    int i, len = 0;

    if((int16_t)val < 0) {
        val = ~val + 1;
    }
    for(i = 15; i >= 0; i--) {
        if((val & (1 << i)) != 0)
            break;
    }
    len += i;
    return len + 1;
}

/**
 * @brief Get minimum number of bits needed to represent the number without sign
 * 
 * @param[in] val   Value as unsigned integer
 * @param[in] signed    Whether or not to consider first bit as sign (discarded)
 *
 * @return minimum number of bits, i.e. index of highest 1
*/
uint8_t min_bits_8(uint8_t val, bool is_signed)
{
    int i, len = 0;

    if((int8_t)val < 0) {
        val = ~val + 1;
    }
    for(i = 7; i >= 0; i--) {
        if((val & (1 << i)) != 0)
            break;
    }
    len += i;
    return len + 1;
}