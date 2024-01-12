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
#include "protocol.h"
#include "channel.h"


tbi_ctx_t *tbi_init(void)
{
    tbi_ctx_t *tbi;
    tbi = malloc(sizeof(tbi_ctx_t));
    if(!tbi) 
        return NULL;
    
    memset(tbi, 0, sizeof(tbi_ctx_t));

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

    if(!tbi || !tbi->channel || tbi->channel->server)
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
    
    if(!tbi ||!tbi->channel || tbi->channel->server)
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
            ret = tbi_serialize_rtm(ctx->format, ctx->msgtype, ctx->format_len, buf_in, len_in, &buf_out, &len_out);
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

/**
 * @brief Blocking receive from client
 * 
 * @param[in] tbi       TBI context
 * 
 * @return number of messages received, 
 *          or a negative error code on failure
*/
int tbi_server_receive_blocking(tbi_ctx_t* tbi)
{
    tbi_msg_ctx_t *ctx;
    uint8_t *buf;
    uint8_t flags, msgtype;
    int len, ret, i;

    if(!tbi || !tbi->channel || !tbi->channel->server)
        return -1;

    /* Receive from client */
    len = tbi_server_channel_recv(tbi);
    if(len <= 0)
        return -1;

    /* Check flags */
    if((ret = tbi_get_client_flags(tbi->channel->buf, &flags, &msgtype)) != 0)
        return -1;


    /* Check for received messages */
    for(i = 0; i < tbi->msg_ctxs_len; i++) {
        ctx = &tbi->msg_ctxs[i];
        if(ctx->msgtype == msgtype) {
            /* Check if msg is RTM or DCB */
            if((flags & TBI_FLAGS_DCB) == TBI_FLAGS_DCB && !ctx->dcb) {
                printf("Unexpected DCB for message type %u!\n", msgtype);
                return -1;
            }
            else if((flags & TBI_FLAGS_RTM) == TBI_FLAGS_RTM && ctx->dcb) {
                printf("Unexpected RTM for message type %u!\n", msgtype);
                return -1;
            }
            
            /* Allocate memory for copying message */
            buf = (uint8_t*)malloc(len * sizeof(uint8_t));
            if(!buf)
                return -1;

            /* Copy message over and store in message buffer */
            memcpy(buf, tbi->channel->buf, len);
            ret = tbi_buf_push_back(ctx, len, buf);
            if(ret != 0) 
                return -1;
            
            /* Signal msg received */
            return 1;
        }
    }
    return 0;
}


/**
 * @brief Process the message buffers, invoking callbacks for received msgs
 * 
 * @param[in] tbi       TBI context
 * 
 * @return number of messages received, 
 *          or a negative error code on failure
*/
int tbi_server_process(tbi_ctx_t* tbi)
{
    tbi_msg_ctx_t * ctx = NULL;
    int i, len_in, len_out, ret;
    uint8_t* buf_out = NULL;
    void* buf_in = NULL;
    int recvd = 0;
    
    if(!tbi || !tbi->channel || !tbi->channel->server)
        return -1;
        
    /* Check for received messages */
    for(i = 0; i < tbi->msg_ctxs_len; i++) {
        ctx = &tbi->msg_ctxs[i];
        if(ctx->buflen >= 1 && !ctx->dcb) {

            /* Pull messages from buffer */
            while((ret = tbi_buf_pop_front(ctx, &len_in, &buf_in)) != 0) {

                /* Deserialize to a native byte stream */
                ret = tbi_deserialize_rtm(ctx->format, ctx->format_len, (uint8_t*)buf_in, len_in, (void**)&buf_out, &len_out);
                if(ret != 0) {
                    free(buf_in);
                    continue;
                }

                /* Invoke callback if set. Global callback has higher precedence */
                if(tbi->global_cb) {
                    tbi->global_cb(ctx->msgtype, buf_out, tbi->global_cb_userdata);
                } else if(ctx->cb) {
                    ctx->cb(ctx->msgtype, buf_out, ctx->cb_userdata);
                } 
                free(buf_in);
                free(buf_out);
                recvd++;
            }
        }
    }

    return recvd;
}

/**
 * @brief Register callback for receiving _ALL_ messages
 * 
 * @param[in] tbi       TBI context
 * @param[in] cb        Callback
 * @param[in] userdata  Optional user context passed to callback
 * 
*/
void tbi_server_register_global_callback(tbi_ctx_t* tbi, tbi_msg_callback cb, void* userdata)
{
    if(!tbi) return;
    
    /* Register global callback, will override msg callbacks */
    tbi->global_cb = cb;
    tbi->global_cb_userdata = userdata;

}

/**
 * @brief Register callback for receiving certain type messages
 * 
 * @param[in] tbi       TBI context
 * @param[in] msgtype   Message type to associate the callback with
 * @param[in] cb        Callback
 * @param[in] userdata  Optional user context passed to callback
 * 
*/
void tbi_server_register_msg_callback(tbi_ctx_t* tbi, uint8_t msgtype, tbi_msg_callback cb, void* userdata)
{
    tbi_msg_ctx_t *ctx;
    int i;

    if(!tbi) return;

    /* Find message type from contexts and register cb */
    for(i = 0; i < tbi->msg_ctxs_len; i++) {
        ctx = &tbi->msg_ctxs[i];
        if(ctx->msgtype == msgtype) {
            ctx->cb = cb;
            ctx->cb_userdata = userdata;
            break;
        }
    }
}


void tbi_close(tbi_ctx_t* tbi)
{
    if(!tbi) return;

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