/**
* @file     serializer.h
* @brief    Header file for message serializer
*/

#ifndef __TBI_SERIALIZER_H
#define __TBI_SERIALIZER_H

#include <stdint.h>


int tbi_serialize_rtm(const uint8_t* msgspec, int spec_len, void *in_buf, int in_len, uint8_t **out_buf, int *out_len);
int tbi_deserialize_rtm(const uint8_t* msgspec, int spec_len, uint8_t *in_buf, int in_len, void** out_buf, int *out_len);

#endif /* __TBI_SERIALIZER_H */