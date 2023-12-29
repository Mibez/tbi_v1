#ifndef __TBI_TYPES_H
#define __TBI_TYPES_H

#include <stdint.h>

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
};

/** @brief Telemetry context for each message type, including a buffer */
typedef struct {
  int msgtype;
  int format_len;
  const uint8_t * format;
  int buflen;
  struct tbi_msg_node *head;
} tbi_msg_ctx_t;

#endif /* __TBI_TYPES_H */