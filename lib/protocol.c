/**
* @file     protocol.c
* @brief    TBI protocol de/serialization implementation
*/

#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>
#include "protocol.h"
#include "utils.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/** @brief Set flags to an outgoing message
 * 
 * @param[out]  buf     Buffer where the flags will be set in the first byte
 * @param[in]   flags   The flags to set
 * 
 * @return 0 on success, or a negative error value
 */
int tbi_set_client_flags(uint8_t *buf, uint8_t flags)
{
    if(!buf) return -1;
    *buf |= (flags << 4);
    return 0;
}

/** @brief Get flags and message type from an incoming message
 * 
 * @param[in]   buf         Buffer where the flags will be set in the first byte
 * @param[out]  flags       Flags from message
 * @param[out]  msgtype     Message type from message
 * 
 * @return 0 on success, or a negative error value
 */
int tbi_get_client_flags(uint8_t *buf, uint8_t *flags, uint8_t *msgtype)
{
    if(!buf) return -1;
    *flags = ((*buf) >> 4) & 0xF;
    *msgtype = (*buf) & 0xF;
    return 0;
}



/** @brief Form the client handshake message
 * 
 * @param[out] buf              Buffer to write the outgoing message
 * @param[in] schema_version    Schema version from messagespec
 * @param[in] schema_csum       Machine-readable schema checksum
 * @param[in] ts                Connection start timestamp that future telemetry msgs will be relative to
 * 
 * @return length of bytes written to buf, or negative error value
 */
int tbi_protocol_client_handshake(uint8_t *buf, uint8_t schema_version, uint16_t schema_csum, uint64_t ts)
{
    uint8_t* buf_ptr;
    int len = 0;

    buf_ptr = buf;
    if(!buf_ptr)
        return -1;

    /* TBI magic */
    *buf_ptr++ = 'T';
    *buf_ptr++ = 'B';
    *buf_ptr++ = 'I';
    len += 3;

    /* TBI protocol version */
    *buf_ptr++ = TBI_PROTOCOL_VERSION;
    len++;

    /* Start timestamp, uint64_t big-endian */
    *(uint32_t*)buf_ptr = htonl((uint32_t)((ts >> 32) & 0xFFFFFFFF));
    buf_ptr += sizeof(uint32_t);
    *(uint32_t*)buf_ptr = htonl((uint32_t)(ts & 0xFFFFFFFF));
    buf_ptr += sizeof(uint32_t);
    len += sizeof(uint32_t) * 2;

    /* Schema version */
    *buf_ptr++ = schema_version;
    len++;

    /* Schema checksum */
    *(uint16_t*)buf_ptr = htons(schema_csum);
    len += sizeof(uint16_t);

    return len;

}

/** @brief Verify server handshake acknowledge
 * 
 * @param[in] buf   Server handshake acknowledge message
 * @param[in] len   Server message length
 * 
 * @return 0 on valid handshake, or negative error value
 */
int tbi_protocol_client_verify_handshake_ack(uint8_t *buf, int len)
{
    uint8_t expected_header[] = {'T', 'B', 'I', TBI_PROTOCOL_VERSION};
    int min_len = ARRAY_SIZE(expected_header);
    int i;

    /* Ensure there's enough to read */
    if(len != min_len)
        return -1;
    
    /* Validate acknowledge */
    for(i = 0; i < ARRAY_SIZE(expected_header); i++) {
        if(*buf++ != expected_header[i])
            return -1;
    }

    return 0;
}


/** @brief Verify client handshake message, and form acknowledge message
 * 
 * @param[in,out] buf           Buffer with client message, server response written back in it
 * @param[in] len               Client message length
 * @param[in] schema_version    Schema version from messagespec
 * @param[in] schema_csum       Machine-readable schema checksum
 * @param[out] out_ts           Connection start timestamp form client that future telemetry msgs will be relative to
 * 
 * @return length of bytes written to buf, or negative error value
 */
int tbi_protocol_server_handshake(uint8_t *buf, int len, uint8_t schema_version, uint16_t schema_csum, uint64_t *out_ts)
{
    uint8_t expected_header[] = {'T', 'B', 'I', TBI_PROTOCOL_VERSION};
    uint32_t ts_hi, ts_lo;
    int i, min_len;

    /* Calculate min len of client msg, so that we don't read past the end; header + ts + schema ver + csum */
    min_len = ARRAY_SIZE(expected_header) + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint16_t);
    if(len != min_len)
        return -1;

    /* Verify magic and version, and prepare ACK with identical content */
    for(i = 0; i < ARRAY_SIZE(expected_header); i++) {
        if(*buf++ != expected_header[i])
            return -1;
    }

    /* Client connection start ts, converted to native endianness */
    ts_hi = ntohl(*(uint32_t*)buf);
    buf += sizeof(uint32_t);
    ts_lo = ntohl(*(uint32_t*)buf);
    buf += sizeof(uint32_t);
    *out_ts = (uint64_t)ts_hi << 32 | (uint64_t)ts_lo;

    /* Verify schema version */
    if(*buf++ != schema_version)
        return -1;

    /* Verify checksum */
    if(schema_csum != ntohs(*(uint16_t*)buf))
        return -1;

    return ARRAY_SIZE(expected_header);
}