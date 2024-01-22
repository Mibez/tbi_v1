/**
* @file     client.c
* @brief    Example simple TBI client implementation. Run utils/compose.py on utils/example.json
*           before compiling 
*/

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "tbi.h"
#include "messagespec.h"

/** @brief Flag to stop looping and its sig handler */
static sig_atomic_t stopping = 0;
void sig_handler(int signum) {
    stopping = 1;
}

int main(int argc, char* arv[])
{
    tbi_ctx_t* tbi;
    int ret;

    signal(SIGINT, sig_handler); 

    tbi = tbi_init();
    if(!tbi)
        return 1;

    printf("Registering message spec...\n");
    if((ret = tbi_register_msgspec(tbi)) != 0)
        goto exit_init;

    printf("Client init...\n");
    if((ret = tbi_client_init(tbi)) != 0)
        goto exit_init;

    /* Telemetry can be either dynamically or statically allocated, it's copied to internal buffers */
    msgspec_temp_and_hum_t *temp1 = (msgspec_temp_and_hum_t *)malloc(sizeof(msgspec_temp_and_hum_t));
    msgspec_temp_and_hum_t temp2 = {
        .time_s = 0xdeadbeef,
        .temp = 0xbaad1dea,
        .hum  = 0xff,
    };
    
    temp1->time_s = 0xaabbccdd;
    temp1->temp = 0x11223344;
    temp1->hum = 0xff;

    printf("Scheduling telemetry...\n");
    if((ret = tbi_send_temp_and_hum(tbi, temp1)) != 0)
        goto exit_alloc;

    if((ret = tbi_send_temp_and_hum(tbi, &temp2)) != 0)
        goto exit_alloc;

    free(temp1);


    /* Schedule a lot of messages for DCB */
    msgspec_acceleration_t acc;
    acc = (msgspec_acceleration_t){.time_s = 0, .time_ms = 900, .acc_x = 777, .acc_y = 6666, .acc_z = 1 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;
    
    acc = (msgspec_acceleration_t){.time_s = 10, .time_ms = 654, .acc_x = 8096, .acc_y = 7777, .acc_z = 2 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;

    acc = (msgspec_acceleration_t){.time_s = 18, .time_ms = 322, .acc_x = 999, .acc_y = 200, .acc_z = -3 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;

    acc = (msgspec_acceleration_t){.time_s = 100, .time_ms = 1, .acc_x = 100, .acc_y = 5656, .acc_z = 4 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;

    acc = (msgspec_acceleration_t){.time_s = 199, .time_ms = 40, .acc_x = 999, .acc_y = 5555, .acc_z = 5 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;

    acc = (msgspec_acceleration_t){.time_s = 250, .time_ms = 888, .acc_x = 999, .acc_y = 6666, .acc_z = 6 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;

    acc = (msgspec_acceleration_t){.time_s = 300, .time_ms = 555, .acc_x = 999, .acc_y = 7777, .acc_z = 7 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;

    acc = (msgspec_acceleration_t){.time_s = 356, .time_ms = 123, .acc_x = 999, .acc_y = -8888, .acc_z = -8 };
    if((ret = tbi_send_acceleration(tbi, &acc)) != 0)
        goto exit_alloc;

    printf("Calling process() to send telemetry...\n");

    while(!stopping) {
        ret = tbi_client_process(tbi);
        usleep(100*1000);
    }

    tbi_close(tbi);

    return 0;

exit_alloc:
    free(temp1);
exit_init:
    tbi_close(tbi);
    return 1;
}