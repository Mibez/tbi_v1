/**
* @file     buf.c
* @brief    TBI telemetry message buffer implementation
*/

#include <stdlib.h>
#include "tbi_types.h"
#include "buf.h"

/**
 * @brief Insert a message to end of list
 * 
 * @param[in] msg_ctx   Telemetry message buffer context for a message type
 * @param[in] buflen    Length of buffer to be stored
 * @param[in] buf       Pointer to buffer to be stored
 * 
 * @return 0 on success, negative error code on failure
*/
int tbi_buf_push_back(tbi_msg_ctx_t *msg_ctx, int buflen, void* buf)
{
    struct tbi_msg_node *new;
    struct tbi_msg_node *curr;
    int index;

    /* Allocate memory for new message node */
    new = (struct tbi_msg_node*)malloc(sizeof(struct tbi_msg_node));
    if(new == NULL) return -1;

    /* Update metadata */
    new->len = buflen;
    new->buf = buf;
    new->next = NULL;

    if(msg_ctx->buflen == 0 || msg_ctx->head == NULL) {
        /* New node is the first element, replace head */
        msg_ctx->head = new;
        msg_ctx->buflen = 1;
    } else {
        /* Insert as last element */
        curr = msg_ctx->head;
        while(curr->next != NULL) {
            curr = curr->next;
            index++;
        }
        curr->next = new;
        msg_ctx->buflen++;

    }

    return 0;
}

/**
 * @brief Retrieve first message in the list
 * 
 * @param[in]   msg_ctx   Telemetry message buffer context for a message type
 * @param[out]  buflen    Length of buffer retrieved
 * @param[out]  buf       Pointer to address of retrieved buffer
 * 
 * @return 0 on success, negative error code on failure
*/
int tbi_buf_pop_front(tbi_msg_ctx_t *msg_ctx, int *buflen, void** buf)
{
    struct tbi_msg_node *first;

    /* Return error if list empty */
    if(msg_ctx->buflen == 0 || msg_ctx->head == NULL)
        return -1;

    /* Change head to point to the second element (may be NULL) */
    first = msg_ctx->head;
    *buflen = first->len;
    *buf = first->buf;
    msg_ctx->head = first->next;
    
    /* Decrement buffer length and free memory taken by old first element */
    msg_ctx->buflen--;
    free(first);

    return 0;
}

/**
 * @brief Free up memory used by message buffer
 * 
 * @param[in]   msg_ctx   Telemetry message buffer context for a message type
 */
void tbi_buf_free(tbi_msg_ctx_t *msg_ctx)
{
    int buflen;
    void *buf;

    while(tbi_buf_pop_front(msg_ctx, &buflen, &buf) == 0) {
        free(buf);
    }
}
