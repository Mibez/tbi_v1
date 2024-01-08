#include <stdio.h>
#include <stdlib.h>

#include "tbi.h"
#include "messagespec.h"


int main(int argc, char* arv[])
{
    tbi_ctx_t* tbi;
    int ret;
    
    tbi = tbi_init();
    if(!tbi)
        return 1;

    printf("Registering message spec...\n");
    if((ret = tbi_register_msgspec(tbi)) != 0)
        return 1;

    printf("Client init...\n");
    if((ret = tbi_client_init(tbi)) != 0)
        return 1;

    msgspec_temp_and_hum_t *temp1 = (msgspec_temp_and_hum_t *)malloc(sizeof(msgspec_temp_and_hum_t));
    msgspec_temp_and_hum_t *temp2 = (msgspec_temp_and_hum_t *)malloc(sizeof(msgspec_temp_and_hum_t));
    
    temp1->time = 0xaabbccdd;
    temp1->temp = 0x11223344;
    temp1->hum = 0xff;

    temp2->time = 0xdeadbeef;
    temp2->temp = 0xbaad1dea;
    temp2->hum = 0x09;

    printf("Scheduling telemetry...\n");
    if((ret = tbi_send_temp_and_hum(tbi, temp1)) != 0)
        return 1;

    if((ret = tbi_send_temp_and_hum(tbi, temp2)) != 0)
        return 1;

    printf("Calling process() to send telemetry...\n");
    while((ret = tbi_client_process(tbi)) > 0) {;}

    tbi_close(tbi);

    return 0;
}