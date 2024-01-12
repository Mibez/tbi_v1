/**
* @file     tbi.h
* @brief    Header file for the TBI library
*/

#ifndef __TBI_H
#define __TBI_H

#include <stdbool.h>
#include "tbi_types.h"


tbi_ctx_t *tbi_init(void);
int tbi_client_init(tbi_ctx_t* tbi);
int tbi_server_init(tbi_ctx_t* tbi);

int tbi_telemetry_schedule(tbi_ctx_t* tbi, int msg_type, const void* buf, int len);

int tbi_client_process(tbi_ctx_t* tbi);

int tbi_server_receive_blocking(tbi_ctx_t* tbi);
int tbi_server_process(tbi_ctx_t* tbi);

void tbi_server_register_global_callback(tbi_ctx_t* tbi, tbi_msg_callback cb, void* userdata);
void tbi_server_register_msg_callback(tbi_ctx_t* tbi, uint8_t msgtype, tbi_msg_callback cb, void* userdata);

void tbi_close(tbi_ctx_t* tbi);

#endif /* __TBI_H */