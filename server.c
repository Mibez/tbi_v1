/**
* @file     server.c
* @brief    Example simple TBI server implementation. Run utils/compose.py on utils/example.json
*           before compiling 
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 

#include "tbi.h"
#include "messagespec.h"

/** @brief Flag to stop looping and its sig handler */
static sig_atomic_t stopping = 0;
void sig_handler(int signum) {
    stopping = 1;
}

/** @brief Example context to be passed back to us in callbacks */
typedef struct {
    uint32_t magic;
} example_server_ctx;

/** @brief Example callback for TEMP_AND_HUM messagetype */
void receive_temp_and_hum(const int msgtype, const void* buf, void* userdata) 
{
    const msgspec_temp_and_hum_t* th = (const msgspec_temp_and_hum_t*)buf;
    example_server_ctx *ctx = (example_server_ctx*)userdata;

    if(msgtype != TEMP_AND_HUM)
        return;
    
    th = (msgspec_temp_and_hum_t*)buf;
    printf("Received temperature and humidity!:\n");
    printf("    time: %d (0x%X)\n", th->time_s, th->time_s);
    printf("    temp: %d (0x%X)\n", th->temp, th->temp);
    printf("    temp: %d (0x%X)\n", th->hum, th->hum);
    printf("            Server magic: 0x%X\n\n", ctx->magic);
}


int main(int argc, char* arv[])
{
    example_server_ctx ctx = {.magic = 0xDEADBEEF};
    tbi_ctx_t* tbi;
    int ret;
    
    signal(SIGINT, sig_handler); 

    tbi = tbi_init();
    if(!tbi)
        return 1;

    printf("Registering message spec...\n");
    if((ret = tbi_register_msgspec(tbi)) != 0)
        return 1;

    printf("Server init...\n");
    if((ret = tbi_server_init(tbi)) != 0) {
        tbi_close(tbi);
        return 1;
    }

    printf("Registering callback(s)...\n");
    tbi_server_register_msg_callback(tbi, TEMP_AND_HUM, &receive_temp_and_hum, &ctx);

    printf("Entering main loop...\n");
    while(!stopping) {
        /* Blocking receive, returns when telemetry received */
        ret = tbi_server_receive_blocking(tbi);
        if(ret > 0) {
            /* Process received messages (invokes callbacks) */
            ret = tbi_server_process(tbi);
            if(ret < 0) {
                printf("Error in process: %d\n", ret);
            }
        } else if (ret < 0) {
            printf("Error in recv: %d\n", ret);
            break;
        }
    }

    tbi_close(tbi);
    return 0;
}