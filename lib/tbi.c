/**
* @file     tbi.c
* @brief    TBI library main interface implemenetation
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tbi_types.h"
#include "tbi.h"
#include "buf.h"
#include "serializer.h"
#include "channel.h"


tbi_ctx_t *tbi_init(void)
{
    tbi_ctx_t *tbi;
    tbi = malloc(sizeof(tbi_ctx_t));
    return tbi;
}

int tbi_client_init(tbi_ctx_t* tbi)
{
    return tbi_client_channel_open(tbi);
}

int tbi_server_init(tbi_ctx_t* tbi)
{
    return tbi_server_channel_open(tbi);
}

/**
 * @brief Schedule a new telemetry message, storing it into 
 * dedicated buffer for sending
 * 
 * @param[in] tbi       TBI context
 * @param[in] msg_type  Message type @ref msgspec_types_t
 * @param[in] buf       Message content
 * 
 * @return 0 on success, negative error code on failure
*/
int tbi_telemetry_schedule(tbi_ctx_t* tbi, int msg_type, const void* buf, int len)
{
    tbi_msg_ctx_t * ctx = NULL;
    uint8_t *msg_copy;
    int i;

    if(!tbi->channel || tbi->channel->server)
        return -1;


    /* Find the correct context for this message type */
    for(i = 0; i < tbi->msg_ctxs_len; i++) {
        ctx = &tbi->msg_ctxs[i];
        if(ctx->msgtype == msg_type) {

            /* Input size must match expected */
            if(len != ctx->raw_size)
                return -1;

            /* Copy message from user to new buffer */
            msg_copy = (uint8_t*)malloc(sizeof(uint8_t)*len);
            memcpy(msg_copy, buf, len);

            /* Store into dedicated buffer */
            return tbi_buf_push_back(ctx, ctx->raw_size, msg_copy);
        }
    }
    
    /* Return error if not found */
    return -1;
}

/**
 * @brief Process the message buffers, looking for any messages to be sent
 * 
 * @param[in] tbi       TBI context
 * 
 * @return number of messages sent, 
 *          or a negative error code on failure
*/
int tbi_client_process(tbi_ctx_t* tbi)
{
    tbi_msg_ctx_t * ctx = NULL;
    int i, len_in, len_out, ret;
    uint8_t* buf_out = NULL;
    void* buf_in = NULL;
    
    if(!tbi->channel || tbi->channel->server)
        return -1;
        
    /* Check for messages to send */
    for(i = 0; i < tbi->msg_ctxs_len; i++) {
        ctx = &tbi->msg_ctxs[i];
        if(ctx->buflen >= 1 && !ctx->dcb) {
            
            /* Pull message from buffer */
            ret = tbi_buf_pop_front(ctx, &len_in, &buf_in);
            if(ret != 0) 
                return ret;

            /* Serialize to a platform-agnostic byte stream */
            ret = tbi_serialize_rtm(ctx->format, ctx->format_len, buf_in, len_in, &buf_out, &len_out);
            free(buf_in);
            if(ret != 0) {
                return ret;
            }

            /* Send message through channel */
            ret = tbi_client_channel_send_rtm(tbi, 0U, ctx->msgtype, buf_out, len_out);
            free(buf_out);
            if(ret != 0) {
                return ret;
            }
            return 1;
        }
    }
    return 0;
}


int tbi_server_process(tbi_ctx_t* tbi)
{
    return 0;
}

void tbi_close(tbi_ctx_t* tbi)
{
    /* Close connection */
    if(tbi->channel) {
        if(tbi->channel->server) {
            tbi_server_channel_close(tbi);
        } else {
            tbi_client_channel_close(tbi);
        }
    }

    /* Clear message buffers */
    for(int i = 0; i < tbi->msg_ctxs_len; i++) {
        tbi_buf_free(&(tbi->msg_ctxs[i]));
    }

    /* Free main context */
    if(tbi) {
        free(tbi);
        tbi = NULL;
    }
}