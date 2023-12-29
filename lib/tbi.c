#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tbi_types.h"
#include "tbi.h"


static tbi_ctx_t* ctx = NULL;


tbi_ctx_t *tbi_init(void)
{
    ctx = malloc(sizeof(tbi_ctx_t));
    /* TODO: init fields */
    return ctx;
}

int tbi_client_init(tbi_ctx_t* tbi)
{
    tbi->server = false;
    return 0;
}

int tbi_server_init(tbi_ctx_t* tbi)
{
    tbi->server = true;
    return 0;
}

int tbi_telemetry_schedule(tbi_ctx_t* tbi, int msg_type, void* buf)
{
    return 0;
}


void tbi_close(tbi_ctx_t* tbi)
{
    if(tbi) {
        free(tbi);
        tbi = NULL;
    }
}