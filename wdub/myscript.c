#include "wdub.h"
#include "yodog.h"

uint8_t *rf(uint8_t*fn)
{
    FILE *fp = NULL;
    uint32_t len = 0;
    uint8_t *d = NULL;

    if (fn == NULL ) { return NULL; }

    fp = fopen((const char *)fn, "rb");

    if ( fp == NULL ) { return NULL;}

    fseek(fp, 0, SEEK_END);

    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    d = malloc( len +1 );

    memset(d, 0x00, len+1);
    fread(d, len, 1, fp);
    fclose(fp);
    return d;
}


int main(int argc, char**argv)
{
    uint8_t outbuff[0x1000];
    uint8_t *d = rf((uint8_t*)argv[1]);

    if ( !d ) {return 0;};

    memset(outbuff, 0x00, 0x1000);
    runscript(d, strlen((const char *)d), outbuff);
    printf("out: %s\n", outbuff);

    return 0;

}
