/**
* @file     protocol.h
* @brief    Header file for TBI protocol de/serialization implementation
*/

#ifndef __TBI_PROTOCOL_H
#define __TBI_PROTOCOL_H

#include <stdint.h>

#define TBI_PROTOCOL_VERSION 1

int tbi_set_client_flags(uint8_t *buf, uint8_t flags);
int tbi_get_client_flags(uint8_t *buf, uint8_t *flags, uint8_t *msgtype);
int tbi_protocol_client_handshake(uint8_t *buf, uint8_t schema_version, uint16_t schema_csum, uint64_t ts);
int tbi_protocol_client_verify_handshake_ack(uint8_t *buf, int len);
int tbi_protocol_server_handshake(uint8_t *buf, int len, uint8_t schema_version, uint16_t schema_csum, uint64_t *out_ts);

#endif /* __TBI_PROTOCOL_H */