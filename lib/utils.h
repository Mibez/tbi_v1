/**
* @file     utils.h
* @brief    Commonly used utility functions
*/

#ifndef __TBI_UTILS_H
#define __TBI_UTILS_H

#include <stdint.h>
#include "tbi_types.h"

/** @brief Get message field type length in bytes
 * 
 * @param[in] field_type    Field type
 * 
 * @return length in bytes
 */
int msg_field_type_len(tbi_msg_field_types_t field_type)
{
    switch (field_type)
    {
        case TBI_TIMEDIFF_S:
        case TBI_TIMEDIFF_MS:
        case TBI_UINT32:
        case TBI_INT32:
            return sizeof(uint32_t);
        case TBI_UINT8:
        case TBI_INT8:
            return sizeof(uint8_t);
        case TBI_UINT16:
        case TBI_INT16:
            return sizeof(uint16_t);
        default:
            return 0;
    }
}

#endif /* __TBI_UTILS_H */