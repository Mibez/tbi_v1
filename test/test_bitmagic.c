/**
* @file     test_bitmagic.c
* @brief    Very basic printf tests for bitmagic.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bitmagic.h"


const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

static void print_byte(uint8_t byte)
{
    printf("%s %s ", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}


static void test_unpack()
{
    // 1101 1110 1010 1101 1011 1110 1110 1111
    uint8_t inbuf[] = {0xDE, 0xAD, 0xBE, 0xEF}; 
    uint32_t res;
    uint32_t printable;
    int bit_position = 32;


    // 1101 1110 1010 1101 1011 1110 1110 1111
    //                                ^------^

    bit_unpack(&inbuf[0], &res, 7, &bit_position);
    if(res != 0x6F) {
        printf("1. Error, expect: 0x6F, got :%X\n", res);
        return;
    } else {
        printf("1. OK! \n");
    }

    // 1101 1110 1010 1101 1011 1110 1110 1111
    //                                ^----^  
    bit_position = 30;
    bit_unpack(&inbuf[0], &res, 5, &bit_position);
    if(res != 0x1B) {
        printf("2. Error, expect: 0x1B, got :%X\n", res);
        return;
    } else {
        printf("2. OK! \n");
    }

    // 1101 1110 1010 1101 1011 1110 1110 1111
    //                   ^-----------------^
    bit_position = 30;
    bit_unpack(&inbuf[0], &res, 15, &bit_position);
    if(res != 0x6FBB) {
        printf("3. Error, expect: 0x6FBB, got :%X\n", res);
        return;
    } else {
        printf("3. OK! \n");
    }

    // 1101 1110 1010 1101 1011 1110 1110 1111
    // ^-----------------------------------^
    bit_position = 30;
    bit_unpack(&inbuf[0], &res, 30, &bit_position);
    if(res != 0x37AB6FBB) {
        printf("4. Error, expect: 0x37AB6FBB, got :%X\n", res);
        return;
    } else {
        printf("4. OK! \n");
    }

    // 1101 1110 1010 1101 1011 1110 1110 1111
    //              ^-------^
    bit_position = 18;
    bit_unpack(&inbuf[0], &res, 7, &bit_position);
    if(res != 0x36) {
        printf("5. Error, expect: 0x36, got: 0x%X\n", res);
        return;
    } else {
        printf("5. OK! \n");
    }
}


static void test_pack() 
{
    uint8_t* resbuf = (uint8_t*)malloc(8);
    uint8_t expect[] = {0xDD, 0x57, 0x1E, 0x20};
    uint32_t inbuf = 0;
    memset(resbuf, 0, sizeof(resbuf));

    int pos = 3; // There's three bits in this byte already
    *resbuf = 0xC0U; // 1100 0000
                     //    ^----
    printf("Running test bit_pack for uint32_t numbers...\n");
    printf("Expectation:\n\n");
    printf("Start:   110                                    Position: 3\n");
    printf("Add 17 : 0001 1101 0101 0111 0001               Position: 20\n");
    printf("Add 7  :                          1110 0010     Position: 27\n");
    printf("Result : 1101 1101 0101 0111 0001 1110 0010 0000\n");

    printf("======================================================================\n\n");

    printf("Reality:\n\n");
    inbuf = (uint32_t)0x1D571; // 0001 1101 0101 0111 0001
    bit_pack(resbuf, &inbuf, 17, &pos);
    printf("Expect: 1101 1101 0101 0111 0001 0000\n");
    printf("Result: ");
    for(int i = 0; i < 2; i++) {print_byte(resbuf[i]);}
    printf("\n=============================================== Position: %d (result)", pos);
    printf("\n\n");


    inbuf = (uint32_t)0x71; // 0111 0001
    bit_pack(resbuf, &inbuf, 7, &pos);
    printf("Expect: 1101 1101 0101 0111 0001 1110 0010 0000\n");
    printf("Result: ");
    for(int i = 0; i < 4; i++) {print_byte(resbuf[i]);}
    printf("\n=============================================== Position: %d (result)", pos);
    printf("\n\n");


    for(int i = 0; i < 4; i++) {
        if(resbuf[i] != expect[i]) {
            printf("Fail in uint32_t version, byte %d\n", i);
            printf(">>>>>>>>>>>>>>>   FAILURE  <<<<<<<<<<<<<<<<<<<<<\n");

            return;
        }
    }
    printf("...OK!\n");
    
    printf("\n\n");
    printf(">>>>>>>>>>>>>>>   SUCCESS  <<<<<<<<<<<<<<<<<<<<<\n");
}

static void test_cross(void)
{
    uint8_t* resbuf = (uint8_t*)malloc(8);
    uint8_t expect[] = {0xDD, 0x57, 0x1E, 0x20};
    uint32_t inbuf = 0;
    uint32_t outbuf;
    int pos_in, pos_out;
    memset(resbuf, 0, sizeof(resbuf));

    pos_in = 3;
    *resbuf = 0xC0U;

    inbuf = (uint32_t)0x1D571; // 0001 1101 0101 0111 0001
    bit_pack(resbuf, &inbuf, 17, &pos_in);
    printf("Expect: 1101 1101 0101 0111 0001 0000\n");
    printf("Result: ");
    for(int i = 0; i < 2; i++) {print_byte(resbuf[i]);}
    printf("\n=============================================== Position: %d (result)\n\n", pos_in);
    
    pos_out = pos_in;
    bit_unpack(resbuf, &outbuf, 17, &pos_out);
    if(pos_out != 3) {
        printf("pos out is not three but %d\n", pos_out);
    } else if(outbuf != inbuf) {
        printf("In/out don't match, in 0x%X, out: 0x%X\n\n", inbuf, outbuf);
    } else {
        printf("OK! in 0x%X, out: 0x%X\n\n", inbuf, outbuf);

    }
    printf("\n\n");


    inbuf = (uint32_t)0x71; // 0111 0001
    //bit_pack(resbuf, inbuf, 32, 7, &pos);
    bit_pack(resbuf, &inbuf, 7, &pos_in);
    printf("Expect: 1101 1101 0101 0111 0001 1110 0010 0000\n");
    printf("Result: ");
    for(int i = 0; i < 4; i++) {print_byte(resbuf[i]);}
    printf("\n=============================================== Position: %d (result)", pos_in);
    printf("\n\n");


    pos_out = pos_in;
    bit_unpack(resbuf, &outbuf, 7, &pos_out);
    if(pos_out != 20) {
        printf("pos out is not 20 but %d\n", pos_out);
    } else if(outbuf != inbuf) {
        printf("In/out don't match, in 0x%X, out: 0x%X\n\n", inbuf, outbuf);
    } else {
        printf("OK! in 0x%X, out: 0x%X\n\n", inbuf, outbuf);

    }
    printf("\n\n");

    for(int i = 0; i < 4; i++) {
        if(resbuf[i] != expect[i]) {
            printf("Fail in uint32_t version, byte %d\n", i);
            printf(">>>>>>>>>>>>>>>   FAILURE  <<<<<<<<<<<<<<<<<<<<<\n");

            return;
        }
    }
    printf("...OK!\n");
    
    printf("\n\n");
    printf(">>>>>>>>>>>>>>>   SUCCESS  <<<<<<<<<<<<<<<<<<<<<\n");
}


int main(void)
{
    test_unpack();
    test_pack();

    test_cross();
}
