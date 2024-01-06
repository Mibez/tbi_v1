/**
* @file     channel.h
* @brief    Header file for socket based channel interface for sending telemetry
*/

#ifndef __TBI_CHANNEL_H
#define __TBI_CHANNEL_H

#include <stdint.h>

int tbi_client_channel_open(void);
int tbi_client_channel_send_rtm(uint8_t flags, uint8_t msgtype, uint8_t* buf, int buf_len);
int tbi_client_channel_send_dcb(void);
int tbi_client_channel_close(void);


#endif /* __TBI_SERIALIZER_H */