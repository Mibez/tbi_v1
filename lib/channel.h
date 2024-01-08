/**
* @file     channel.h
* @brief    Header file for socket based channel interface for sending telemetry
*/

#ifndef __TBI_CHANNEL_H
#define __TBI_CHANNEL_H

#include <stdint.h>
#include "tbi_types.h"


int tbi_client_channel_open(tbi_ctx_t* tbi);
int tbi_client_channel_send_rtm(tbi_ctx_t* tbi, uint8_t flags, uint8_t msgtype, uint8_t* buf, int buf_len);
int tbi_client_channel_send_dcb(tbi_ctx_t* tbi);
void tbi_client_channel_close(tbi_ctx_t* tbi);

int tbi_server_channel_open(tbi_ctx_t* tbi);
int tbi_server_register_cb(tbi_ctx_t* tbi);
void tbi_server_channel_close(tbi_ctx_t* tbi);

#endif /* __TBI_SERIALIZER_H */