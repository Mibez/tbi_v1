/**
* @file     utils.c
* @brief    Commonly used utility functions implementation
*/

#include <sys/time.h>
#include <stdlib.h>

#include "utils.h"
#include "crc16.h"

/** @brief Get message field type length in bytes
 * 
 * @param[in] field_type    Field type
 * 
 * @return length in bytes
 */
int msg_field_type_len(tbi_msg_field_types_t field_type)
{
    switch (field_type)
    {
        case TBI_TIMEDIFF_S:
        case TBI_TIMEDIFF_MS:
        case TBI_UINT32:
        case TBI_INT32:
            return sizeof(uint32_t);
        case TBI_UINT8:
        case TBI_INT8:
            return sizeof(uint8_t);
        case TBI_UINT16:
        case TBI_INT16:
            return sizeof(uint16_t);
        default:
            return 0;
    }
}

/** @brief Compute a checksum for the message spec
 * 
 * @param[in] tbi    tbi context
 * 
 * @return CRC16-CCITT of message spec
 */
uint16_t msgspec_checksum(tbi_ctx_t* tbi)
{
    tbi_msg_ctx_t *msg_ctx;
    uint16_t crc;
    int i, j;

    crc = crc16_begin();

    /* Compute checksum of message type IDs, and the format arrays */
    for(i = 0; i < tbi->msg_ctxs_len; i++) {
        msg_ctx = &(tbi->msg_ctxs[i]);
        crc = crc16(crc, msg_ctx->msgtype);
        for(j = 0; j < msg_ctx->format_len; j++) {
            crc = crc16(crc, msg_ctx->format[j]);
        }
    }

    return crc;
}

/** @brief Get current time in milliseconds since epoch */
uint64_t get_current_time_ms(void)
{
    struct timeval now;
    uint64_t ms = 0U;
    int ret;

    if((ret = gettimeofday(&now, NULL)) == 0) {
        ms = (uint64_t)(now.tv_sec * 1000U) + (uint64_t)(now.tv_usec / 1000U);
    }

    return ms;

}