/**
* @file     serializer.c
* @brief    Message (de)serializer implementation
*/
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "serializer.h"
#include "utils.h"
#include "bitmagic.h"

/** @brief Serialize RTM message to a byte stream in a platform-agnostic manner
 * 
 * @param[in] msgspec   Binary message spec for given message type
 * @param[in] spec_len  Binary message spec length
 * @param[in] msgtype   Message type
 * @param[in] in_buf    Buffer to serialize
 * @param[in] in_len    Buffer to serialize length
 * @param[out] out_buf  Output buffer (must be freed after use)
 * @param[out] out_len  Output buffer length
 * 
 * @return 0 on success, or a negative error code
 */
int tbi_serialize_rtm(const uint8_t* msgspec, int spec_len, uint8_t msgtype, void *in_buf, int in_len, uint8_t **out_buf, int *out_len)
{
    uint8_t *buf;
    uint8_t *in_ptr;
    uint8_t *out_ptr;
    int len = 0;
    int i;

    /* Get total length in bytes and allocate buffer based on it */
    for(i = 0; i < spec_len; i++) {
        len += msg_field_type_len(msgspec[i]);
    }
    len += 1; // Flags & msgtype
    buf = (uint8_t*)malloc(len);
    if(!buf) 
        return -1;

    *out_len = len;
    in_ptr = (uint8_t*)in_buf;
    out_ptr = buf;
    *out_ptr++ = msgtype;

    /* Convert each element to network-endian byte stream based on size */
    for(i = 0; i < spec_len; i++) {
        len = msg_field_type_len(msgspec[i]);
        switch(len) {
            case 4:
            {
                *(uint32_t*)out_ptr = htonl(*(uint32_t*)in_ptr);
                in_ptr = (void*)((uint32_t*)in_ptr + 1);
                out_ptr += sizeof(uint32_t);
                break;
            }
            case 2:
            {
                *(uint16_t*)out_ptr = htons(*(uint16_t*)in_ptr);
                in_ptr = (void*)((uint16_t*)in_ptr + 1);
                out_ptr += sizeof(uint16_t);
                break;
            }
            case 1:
            {
                *out_ptr = *(uint8_t*)in_ptr;
                in_ptr = (void*)((uint8_t*)in_ptr + 1);
                out_ptr += sizeof(uint8_t);
                break;
            }
            default:
                break;
        }
    }

    *out_buf = buf;

    return 0;
}

/** @brief Deserialize RTM message from a platform-agnostic byte stream to native endianness
 * 
 * @param[in] msgspec   Binary message spec for given message type
 * @param[in] spec_len  Binary message spec length
 * @param[in] in_buf    Buffer to serialize
 * @param[in] in_len    Buffer to serialize length
 * @param[out] out_buf  Output buffer (must be freed after use if success returned)
 * @param[out] out_len  Output buffer length
 * 
 * @return 0 on success, or a negative error code
 */
int tbi_deserialize_rtm(const uint8_t* msgspec, int spec_len, uint8_t *in_buf, int in_len, void** out_buf, int *out_len)
{
    uint8_t *buf;
    uint8_t *in_ptr;
    uint8_t *out_ptr;
    int len = 1; // Msgtype & flags
    int i;

    /* Get total length in bytes and allocate buffer based on it */
    for(i = 0; i < spec_len; i++) {
        len += msg_field_type_len(msgspec[i]);
    }

    /* Expected and received buffer size must match exactly */
    if(in_len != len) {
        printf("Deserialize length mismatch! Received: %d, expected: %d bytes\n", in_len, len);
        return -1;
    }

    /* Allocate memory for output buffer */
    buf = (uint8_t*)malloc(len);
    if(!buf)
        return -1;

    *out_len = len;
    in_ptr = in_buf;
    out_ptr = buf;
    in_ptr++; // skip msgtype and flags (1st byte)

    /* Convert network-endian byte stream to native in chunk sizes defined by spec */
    for(i = 0; i < spec_len; i++) {
        len = msg_field_type_len(msgspec[i]);
        switch(len) {
            case 4:
            {
                *(uint32_t*)out_ptr = ntohl(*(uint32_t*)in_ptr);
                out_ptr = (void*)((uint32_t*)out_ptr + 1);
                in_ptr += sizeof(uint32_t);
                break;
            }
            case 2:
            {
                *(uint16_t*)out_ptr = ntohs(*(uint16_t*)in_ptr);
                out_ptr = (void*)((uint16_t*)out_ptr + 1);
                in_ptr += sizeof(uint16_t);
                break;
            }
            case 1:
            {
                *out_ptr = *(uint8_t*)in_ptr;
                out_ptr = (void*)((uint8_t*)out_ptr + 1);
                in_ptr += sizeof(uint8_t);
                break;
            }
            default:
                break;
        }
    }

    *out_buf = (void*)buf;

    return 0;
}

/** @brief Convert current buffer into diff compared to previous buffer in-place, extract sign
 * 
 * @param[in] msgspec       Binary message spec for given message type
 * @param[in] spec_len      Binary message spec length
 * @param[in] previous      Previous buffer to compare to
 * @param[in,out] current   Current value, diff stored here in-place
 * @param[out] signs        Diff signs
 * @param[out] buflen       Input buffer length
 * 
 * @return 0 on success, or a negative error code
 */
static int convert_to_diff(const uint8_t* msgspec, int spec_len, void* previous, void* current, uint8_t* signs, int buflen)
{
    int i, len = 0;
    uint8_t* prev_ptr;
    uint8_t* cur_ptr;
    uint8_t* signs_ptr;
    bool has_sign;

    /* Check length */
    for(i = 0; i < spec_len; i++) {
        len += msg_field_type_len(msgspec[i]);
    }

    if(buflen != len) 
        return -1;

    prev_ptr = previous;
    cur_ptr = current;
    signs_ptr = signs;

    /* Convert to unsigned diff between current and previous value */
    for(i = 0; i < spec_len; i++) {
        len = msg_field_type_len(msgspec[i]);
        has_sign = is_signed(msgspec[i]);
        switch (len)
        {
            case 4:
            {
                if(has_sign) {
                    int32_t cur = *(int32_t*)cur_ptr;
                    int32_t prev = *(int32_t*)prev_ptr;
                    /* Always store diff as an unsigned to maintain bit limit, and sign separately. 
                        (INT32_MAX - INT32_MIN == UINT32_MAX), and rand_uint32_a - rand_uint32_b is negative
                        when rand_uint32_b > rand_uint32_a */
                    /* TODO: check if using overflow is smaller */
                    if(cur < prev) {
                        *signs_ptr++ = 1U;
                        *(uint32_t*)cur_ptr = prev - cur;
                    } else {
                        *signs_ptr++ = 0U;
                        *(uint32_t*)cur_ptr = cur - prev;
                    }
                } else {
                    uint32_t cur = *(uint32_t*)cur_ptr;
                    uint32_t prev = *(uint32_t*)prev_ptr;

                    if(cur < prev) {
                        *signs_ptr++ = 1U;
                        *(uint32_t*)cur_ptr = prev - cur;
                    } else {
                        *signs_ptr++ = 0U;
                        *(uint32_t*)cur_ptr = cur - prev;
                    }
                }
                cur_ptr = (void*)((uint32_t*)cur_ptr + 1);
                prev_ptr = (void*)((uint32_t*)prev_ptr + 1);
                break;
            }
            case 2:
            {
                if(has_sign) {
                    int16_t cur = *(int16_t*)cur_ptr;
                    int16_t prev = *(int16_t*)prev_ptr;
                    if(cur < prev) {
                        *signs_ptr++ = 1U;   
                        *(uint16_t*)cur_ptr = prev - cur;
                    } else {
                        *signs_ptr++ = 0U;
                        *(uint16_t*)cur_ptr = cur - prev;
                    }
                } else {
                    uint16_t cur = *(uint16_t*)cur_ptr;
                    uint16_t prev = *(uint16_t*)prev_ptr;
                    if(cur < prev) {
                        *signs_ptr++ = 1U;   
                        *(uint16_t*)cur_ptr = prev - cur;
                    } else {
                        *signs_ptr++ = 0U;
                        *(uint16_t*)cur_ptr = cur - prev;
                    }
                }
                cur_ptr = (void*)((uint16_t*)cur_ptr + 1);
                prev_ptr = (void*)((uint16_t*)prev_ptr + 1);
                break;
            }
            case 1:
            {
                if(has_sign) {
                    int8_t cur = *(int8_t*)cur_ptr;
                    int8_t prev = *(int8_t*)prev_ptr;
                    if(cur < prev) {
                        *signs_ptr++ = 1U;   
                        *(uint8_t*)cur_ptr = prev - cur;
                    } else {
                        *signs_ptr++ = 0U;
                        *(uint8_t*)cur_ptr = cur - prev;
                    }
                } else {
                    uint8_t cur = *(uint8_t*)cur_ptr;
                    uint8_t prev = *(uint8_t*)prev_ptr;
                    if(cur < prev) {
                        *signs_ptr++ = 1U;   
                        *(uint8_t*)cur_ptr = prev - cur;
                    } else {
                        *signs_ptr++ = 0U;
                        *(uint8_t*)cur_ptr = cur - prev;
                    }
                }
                cur_ptr = (void*)((uint8_t*)cur_ptr + 1);
                prev_ptr = (void*)((uint8_t*)prev_ptr + 1);
                break;
            }
            default:
                break;
        }
    }

    return 0;
}


/** @brief Get minimum number of bits needed to represent a value for a buffer
 * 
 * @param[in] msgspec       Binary message spec for given message type
 * @param[in] spec_len      Binary message spec length
 * @param[out] lens_out     Output bit counts
 * @param[in] buf           Buffer to read values from
 * @param[in] buf_len       Buffer length
 * 
 * @return 0 on success, or a negative error code
 */
static int get_min_bit_repr(const uint8_t* msgspec, int spec_len, uint8_t* lens_out, void* buf, int buf_len)
{
    int i, len = 0;
    bool has_sign;
    uint32_t uint32_val;
    uint16_t uint16_val;
    uint8_t uint8_val;

    uint8_t* buf_ptr;
    uint8_t* out_ptr;

    buf_ptr = buf;
    out_ptr = lens_out;

    /* Get total length in bytes */
    for(i = 0; i < spec_len; i++) {
        len += msg_field_type_len(msgspec[i]);
    }

    if(len != buf_len) 
        return -1;
    
    /* Compute minimum bits needed to represent each element */
    for(i = 0; i < spec_len; i++) {
        len = msg_field_type_len(msgspec[i]);
        switch (len)
        {
        case 4:
            uint32_val = *(uint32_t*)buf_ptr;
            *out_ptr++ = min_bits_32(uint32_val, false);
            buf_ptr += sizeof(uint32_t);
            break;
        
        case 2:
            uint16_val = *(uint16_t*)buf_ptr;
            *out_ptr++ = min_bits_16(uint16_val, false);
            buf_ptr += sizeof(uint16_t);
            break;
        case 1:
            uint8_val = *(uint8_t*)buf_ptr;
            *out_ptr++ = min_bits_8(uint8_val, false);
            buf_ptr += sizeof(uint8_t);
            break;

        default:
            break;
        }
    }

    return 0;
}


/** @brief Serialize DCB message to a byte stream using as little bytes as possible
 * 
 * @param[in] msgspec       Binary message spec for given message type
 * @param[in] spec_len      Binary message spec length
 * @param[in] msgtype       Message type
 * @param[in] in_bufs       Buffers to serialize
 * @param[in] in_buf_lens   Buffers to serialize lengths
 * @param[in] in_len        Number of input buffers
 * @param[out] out_buf      Output buffer
 * @param[out] out_len      Output buffer length
 * 
 * @return 0 on success, or a negative error code
 */
int tbi_serialize_dcb(const uint8_t* msgspec, int spec_len, uint8_t msgtype, void**in_bufs, int* in_buf_lens, int in_len, uint8_t *out_buf, int *out_len)
{
    int ret, i, j, bit_pos, in_buf_len, field_len;
    uint8_t *out_buf_ptr;
    void** in_buf_ptr;
    uint8_t *rtm, *lens_ptr, *signs_ptr;
    uint8_t *saved_prev, *saved_cur;
    uint8_t *maxes;
    uint8_t max_val;
    uint32_t value;
    int len = 0;

    out_buf_ptr = out_buf;
    in_buf_ptr = in_bufs;

    in_buf_len = *in_buf_lens;
    for(i = 1; i < in_len; i++) {
        if(in_buf_lens[i] != in_buf_len)
            return -1;
    }

    lens_ptr = (uint8_t*)malloc(spec_len * (in_len - 1) * sizeof(uint8_t));
    if(!lens_ptr) 
        return -1;

    signs_ptr = (uint8_t*)malloc(spec_len * (in_len - 1) * sizeof(uint8_t));
    if(!signs_ptr) {
        free(lens_ptr);
        return -1;
    }

    /* First, an RTM */
    ret = tbi_serialize_rtm(msgspec, spec_len, msgtype, *in_buf_ptr, in_buf_len, &rtm, &len);
    if(ret != 0 || in_len == 1) {
        return ret;
    }

    /* Copy RTM to final buffer */
    memcpy(out_buf_ptr, rtm, len);
    out_buf_ptr += len;
    free(rtm);
    rtm = NULL;

    /* Convert to diffs in place and gather the diff signs */
    saved_prev = (uint8_t*)malloc(in_buf_len * sizeof(uint8_t));
    saved_cur = (uint8_t*)malloc(in_buf_len * sizeof(uint8_t));
    if(!saved_prev || !saved_cur)
        return -1;

    memcpy(saved_prev, *in_buf_ptr, in_buf_len);
    for(i = 1; i < in_len; i++) {
        memcpy(saved_cur, in_buf_ptr[i], in_buf_len);
        if((ret = convert_to_diff(msgspec, spec_len, (void*)saved_prev, in_buf_ptr[i], signs_ptr + (spec_len * (i - 1)), in_buf_len)) != 0) {
            free(saved_cur);
            free(saved_prev);
            free(lens_ptr);
            free(signs_ptr);
            // TODO: please make this a goto
            return -1;
        }
        memcpy(saved_prev, saved_cur, in_buf_len);
    }
    free(saved_cur);
    free(saved_prev);


    /* Get minimum bit representation for each message separately */
    for(i = 0; i < in_len - 1; i++) {
        if((ret = get_min_bit_repr(msgspec, spec_len, lens_ptr + (spec_len*i), in_buf_ptr[i + 1], in_buf_len)) != 0)
            return ret;
    }

    /* Find global maximum of bit representation per message element type */
    maxes = (uint8_t*)malloc(spec_len * sizeof(uint8_t));
    if(!maxes) {
        free(lens_ptr);
        free(signs_ptr);
        // GOTO!
        return -1;
    }
    memset(maxes, 0, spec_len * sizeof(uint8_t));
    max_val = 0;
    for(i = 0; i < in_len - 1; i++) {
        for(j = 0; j < spec_len; j++) {
            max_val = *(lens_ptr + (spec_len * i) + j);
            if(maxes[j] < max_val)
                maxes[j] = max_val;
        }
    }
    printf("Max bits per element: \n"); for(j = 0; j < spec_len; j++) {  printf(" %u", maxes[j]); } printf("\n");
    
    /* Store no. of values in current format (now just all) */
    *out_buf_ptr++ = in_len - 1;

    /* Store bits used per member */
    bit_pos = 0;
    for(i = 0; i < spec_len; i++) {
        value = (uint32_t)maxes[i];
        bit_pack(out_buf_ptr, &value, 6, &bit_pos);
    }

    /* Store based on that minimum representation */
    for(i = 0; i < in_len - 1; i++) {

        /* Store diff signs */
        for(j = 0; j < spec_len; j++) {
            value = (uint32_t)(*(signs_ptr + (spec_len * i) + j));
            bit_pack(out_buf_ptr, &value, 1, &bit_pos);
        }

        /* Store diffs */
        uint8_t *diff_ptr = (uint8_t*)in_buf_ptr[i+1];
        for(j = 0; j < spec_len; j++) {
            field_len = msg_field_type_len(msgspec[j]);
            switch(field_len) {
                case 4:
                {
                    value = *(uint32_t*)diff_ptr;
                    bit_pack(out_buf_ptr, &value, maxes[j], &bit_pos);
                    diff_ptr += sizeof(uint32_t);
                    break;
                }
                case 2:
                {
                    value = (uint32_t)(*(uint16_t*)diff_ptr);
                    bit_pack(out_buf_ptr, &value, maxes[j], &bit_pos);
                    diff_ptr += sizeof(uint16_t);
                    break;
                }
                case 1:
                {
                    value = (uint32_t)(*(uint8_t*)diff_ptr);
                    bit_pack(out_buf_ptr, &value, maxes[j], &bit_pos);
                    diff_ptr++;
                    break;
                }
            }
        }
    }
    printf("\nDelta compression done! Total bytes (compressed): %d, total bytes (uncompressed): %d\n", (bit_pos / 8) + 1, (in_len-1)* in_buf_len);

    free(lens_ptr);
    free(signs_ptr);
    free(maxes);

    /* Calculate final length */
    len += bit_pos / 8;
    if((bit_pos % 8) != 0)
        len++;

    *out_len = len;
    return 0;

}
