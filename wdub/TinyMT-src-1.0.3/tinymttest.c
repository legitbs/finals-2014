#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include "tinymt32.h"

uint32_t b = 0;

int main(int argc, char**argv)
{
    int i = 1;
    uint32_t bp = (uint32_t)&b;

    tinymt32_t tinymt;
    tinymt.mat1 = 0x1337;
    tinymt.mat2 = 0x4ac04;
    tinymt.tmat = 0x7331;

    uint32_t bone = (bp)&0xff;
    uint32_t btwo = ((bp)>>8)&0xff;
    uint32_t bthr = ((bp)>>16)&0xff;
    uint32_t bfor = ((bp)>>24)&0xff;

    tinymt32_init(&tinymt, bone);
    printf("Seed: %x\nRand: %x\n", bone, tinymt32_generate_uint32(&tinymt));
    tinymt32_init(&tinymt, btwo);
    printf("Seed: %x\nRand: %x\n", btwo, tinymt32_generate_uint32(&tinymt));
    tinymt32_init(&tinymt, bthr);
    printf("Seed: %x\nRand: %x\n", bthr, tinymt32_generate_uint32(&tinymt));
    tinymt32_init(&tinymt, bfor);
    printf("Seed: %x\nRand: %x\n", bfor, tinymt32_generate_uint32(&tinymt));


    return 1;
}
