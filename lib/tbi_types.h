/**
* @file     tbi_types.h
* @brief    Commonly used type definitions for the TBI library
*/

#ifndef __TBI_TYPES_H
#define __TBI_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Type for storing time difference of full seconds, 32-bit */
typedef uint32_t timediff_s;

/** @brief Type for storing time difference of seconds and milliseconds, in 32 bits*/
typedef struct {
  uint32_t seconds: 22;  
  uint32_t ms: 10;  
} timediff_ms;

/** @brief Telemetry message linked list element */
typedef struct tbi_msg_node {
  int   len;
  void *buf;
  struct tbi_msg_node * next;
} tbi_msg_node;

/** @brief Binary message format types */
typedef enum uint8_t {
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
  int msgtype;                /** @brief Message type @ref msgspec_types_t */
  bool dcb;                   /** @brief Should these messages be bundled or not */
  int raw_size;               /** @brief Message size when storing into buffer */
  int format_len;             /** @brief Size of the binary message format specifier */
  const uint8_t * format;     /** @brief Array of @ref tbi_msg_field_types_t for this format */
  int buflen;                 /** @brief Number of items in the message buffer */
  struct tbi_msg_node *head;  /** @brief Pointer to first element in the message buffer */
} tbi_msg_ctx_t;

#endif /* __TBI_TYPES_H */