/**
* @file     tbi_types.h
* @brief    Commonly used type definitions for the TBI library
*/

#ifndef __TBI_TYPES_H
#define __TBI_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define TBI_FLAGS_NONE  (0)
#define TBI_FLAGS_RTM   (1)
#define TBI_FLAGS_DCB   (1 << 1)

/** @brief Message reception callback. 
 * Will be called with message type, the message itself (must be copied
 * to user context), and optional user context
 */
typedef void(*tbi_msg_callback)(const int message_type, const void* msg, void* userdata);


/** @brief Type for storing time difference of full seconds, 32-bit */
typedef uint32_t timediff_s;

/** @brief Type for storing time difference of milliseconds, in 16 bits */
typedef uint16_t timediff_ms;

/** @brief Telemetry message linked list element */
typedef struct tbi_msg_node {
  int   len;
  void *buf;
  struct tbi_msg_node * next;
} tbi_msg_node;

/** @brief Channel context */
typedef struct {
    bool server;
    bool connected;
    int conn_fd;
    int listen_fd;
    uint64_t start_ts;
    uint8_t *buf;
} tbi_channel_t;


/** @brief Binary message format types */
typedef enum {
  TBI_TIMEDIFF_S  = 0,
  TBI_TIMEDIFF_MS = 1,
  TBI_UINT8       = 2,
  TBI_INT8        = 3,
  TBI_UINT16      = 4,
  TBI_INT16       = 5,
  TBI_UINT32      = 6,
  TBI_INT32       = 7,
} tbi_msg_field_types_t;

/** @brief Telemetry context for each message type, including a buffer */
typedef struct {
  uint8_t msgtype;            /** @brief Message type @ref msgspec_types_t */
  bool dcb;                   /** @brief Should these messages be bundled or not */
  int raw_size;               /** @brief Message size when storing into buffer */
  int format_len;             /** @brief Size of the binary message format specifier */
  const uint8_t * format;     /** @brief Array of @ref tbi_msg_field_types_t for this format */
  int buflen;                 /** @brief Number of items in the message buffer */
  struct tbi_msg_node *head;  /** @brief Pointer to first element in the message buffer */
  tbi_msg_callback cb;        /** @brief Message reception callback for this message type */
  void* cb_userdata;          /** @brief Optional user context associated with the callback */
  uint64_t interval;          /** @brief Send interval in ms for DCB messages */
  uint64_t last_sent_ms;      /** @brief Time when this msgtype was last sent */
} tbi_msg_ctx_t;

/** @brief Main TBI library context data structure */
typedef struct {
    uint8_t msgspec_version;
    int msg_ctxs_len;
    tbi_msg_ctx_t *msg_ctxs;
    tbi_channel_t *channel;
    tbi_msg_callback global_cb;
    void* global_cb_userdata;
} tbi_ctx_t;

#endif /* __TBI_TYPES_H */