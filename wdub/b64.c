#include "b64.h"

uint32_t encodeblock( uint8_t *in, uint8_t*out, uint32_t len )
{
    register uint32_t cnt = 0;

    if ( in == 0 )
    {
        goto end;
    }

    if ( out == 0 )
    {
        goto end;
    }


    // upper six bits of the first byte
    out[0] = b64t[ in[0] >> 2 ];
    cnt++;

    // lower two and upper four of next
    out[1] = ( (len==1) ? b64t[ ((in[0]&0x3)<<4)] : b64t[ ((in[0] & 3) << 4 ) | (( in[1] & 0xf0 ) >> 4)] );
    cnt++;

    out[2] = (len > 1 ) ? b64t[ ((in[1] & 0x0f ) << 2) | ( (in[2] &0xc0)>>6)] : '=';
    cnt++;

    out[3] = ((len > 2 ) ? b64t[ ((in[2] & 0x3f))] : '=');
    cnt++;

end:
    return cnt;
    
}

uint8_t * b64encode( uint8_t*in, uint32_t len )
{
    register uint8_t* out = NULL;
    register uint32_t tlen = (len*8)/ 6;
    register uint32_t icnt = 0;
    register uint32_t ocnt = 0;

    if ( in == NULL )
    {
        goto end;
    }

    // align to the next highest 4 byte boundary
    tlen += 3;
    tlen &= 0xfffffffc;

    out = (uint8_t*)malloc(tlen+1);

    if ( out == NULL )
    {
        goto end;
    }

    memset(out, 0x00, tlen +1);

    while ( icnt < len)
    {
        encodeblock( in+icnt, out+ocnt, len - icnt );
        icnt += 3;
        ocnt += 4;    
    }

end:
    return out;
}
