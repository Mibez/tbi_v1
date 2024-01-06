/**
* @file     tbi.h
* @brief    Header file for the TBI library
*/

#ifndef __TBI_H
#define __TBI_H

#include <stdbool.h>
#include "tbi_types.h"

typedef struct {
    bool server;
    int msg_ctxs_len;
    tbi_msg_ctx_t *msg_ctxs;
} tbi_ctx_t;


tbi_ctx_t *tbi_init(void);
int tbi_client_init(tbi_ctx_t* tbi);
int tbi_server_init(tbi_ctx_t* tbi);

int tbi_telemetry_schedule(tbi_ctx_t* tbi, int msg_type, void* buf);

int tbi_client_process(tbi_ctx_t* tbi);
int tbi_server_process(tbi_ctx_t* tbi);

void tbi_close(tbi_ctx_t* tbi);

#endif /* __TBI_H */