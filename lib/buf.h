/**
* @file     buf.h
* @brief    Header file for TBI telemetry message buffer implementation
*/

#ifndef __TBI_BUF_H
#define __TBI_BUF_H

#include "tbi_types.h"

int tbi_buf_push_back(tbi_msg_ctx_t *msg_ctx, int buflen, void* buf);
int tbi_buf_pop_front(tbi_msg_ctx_t *msg_ctx, int *buflen, void** buf);

void tbi_buf_free(tbi_msg_ctx_t *msg_ctx);

#endif /* __TBI_BUF_H */