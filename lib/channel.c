/**
* @file     channel.c
* @brief    Socket based channel interface for sending telemetry
*/

#include <stdio.h>

#include "channel.h"


int tbi_client_channel_open(void)
{
    printf("DUMMY channel open!\n");
    return 0;
}

int tbi_client_channel_send_rtm(uint8_t flags, uint8_t msgtype, uint8_t* buf, int buf_len)
{
    printf("DUMMY channel send RTM!\n");
    for(int i = 0; i < buf_len; i++)
    {
        printf("0x%X ", buf[i]);
    }
    printf("\n");
    return 0;
}

int tbi_client_channel_send_dcb(void)
{
    printf("DUMMY channel send DCB!\n");
    return 0;
}

int tbi_client_channel_close(void)
{
    printf("DUMMY channel close!\n");
    return 0;
}