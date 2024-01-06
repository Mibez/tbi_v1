/**
* @file     serializer.c
* @brief    Message (de)serializer implementation
*/
#include <netinet/in.h>
#include <stdlib.h>
#include "serializer.h"
#include "utils.h"

/** @brief Serialize RTM message to a byte stream in a platform-agnostic manner
 * 
 * @param[in] msgspec   Binary message spec for given message type
 * @param[in] spec_len  Binary message spec length
 * @param[in] in_buf    Buffer to serialize
 * @param[in] in_len    Buffer to serialize length
 * @param[out] out_buf  Output buffer (must be freed after use)
 * @param[out] out_len  Output buffer length
 * 
 * @return 0 on success, or a negative error code
 */
int tbi_serialize_rtm(const uint8_t* msgspec, int spec_len, void *in_buf, int in_len, uint8_t **out_buf, int *out_len)
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
    buf = (uint8_t*)malloc(len);
    if(!buf) 
        return -1;

    *out_len = len;
    in_ptr = (uint8_t*)in_buf;
    out_ptr = buf;

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
 * @param[out] out_buf  Output buffer (must be freed after use)
 * @param[out] out_len  Output buffer length
 * 
 * @return 0 on success, or a negative error code
 */
int tbi_deserialize_rtm(const uint8_t* msgspec, int spec_len, uint8_t *in_buf, int in_len, void** out_buf, int *out_len)
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

    /* Expected and received buffer size must match exactly */
    if(in_len != len) 
        return -1;

    /* Allocate memory for output buffer */
    buf = (uint8_t*)malloc(len);
    if(!buf)
        return -1;

    *out_len = len;
    in_ptr = in_buf;
    out_ptr = buf;

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