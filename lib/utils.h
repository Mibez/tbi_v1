/**
* @file     utils.h
* @brief    Commonly used utility functions
*/

#ifndef __TBI_UTILS_H
#define __TBI_UTILS_H

#include <stdint.h>
#include "tbi_types.h"

int msg_field_type_len(tbi_msg_field_types_t field_type);
bool is_signed(tbi_msg_field_types_t field_type);

uint16_t msgspec_checksum(tbi_ctx_t* tbi);
uint64_t get_current_time_ms(void);

#endif /* __TBI_UTILS_H */