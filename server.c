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

    printf("Server init...\n");
    if((ret = tbi_server_init(tbi)) != 0) {
        tbi_close(tbi);
        return 1;
    }

    tbi_close(tbi);
    return 0;
}