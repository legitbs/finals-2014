#include "wdub.h"

uint32_t AddSetCookie( pstr strg )
{
    register uint32_t retval = 0;
    register uint32_t length = 0;
    #ifdef x86_64
    register uint64_t addr = (uint64_t)&AddSetCookie;
    #else
    register uint32_t addr = (uint32_t)&AddSetCookie;
    #endif

    register uint32_t seed = 0;
    uint32_t randval = 0;
    uint8_t *b64stuff = NULL;

    tinymt32_t tinymt;

    if ( strg == NULL )
    {
        goto end;
    }

    retval = append_str( strg, (uint8_t*)"Shouts: Saito and Matsumoto\n\r");

    if ( !retval )
        goto end;

    retval = append_str( strg, (uint8_t*)"Set-Cookie: " );

    if ( !retval)
        goto end;

    tinymt.mat1 = 0x6954;
    tinymt.mat2 = 0x796e;
    tinymt.tmat = 0x746d;

    length = sizeof(addr);

    while ( length )
    {
        seed = addr&0xff;
        tinymt32_init(&tinymt, seed);
        randval = tinymt32_generate_uint32(&tinymt);
        addr >>= 8;
        length--;

        b64stuff = b64encode( (uint8_t*)&randval, sizeof(randval ) );

        if ( b64stuff == NULL )
        {
            retval = 0;
            goto end;
        }

        retval = append_str( strg, b64stuff );

        if ( !retval)
        {
            goto end;
        }

        if ( length ) {
            retval = append_str( strg, (uint8_t*)"::" );

            if ( !retval)
                goto end;
        }

        free( b64stuff );
    }

    retval = append_str( strg, (uint8_t*)"\r\n");

end:
    return retval;
}



uint32_t AddConnection( pstr strg, uint32_t t )
{
    register uint32_t retval = 0;
    register uint8_t *s = NULL;

    if ( strg == NULL )
    {
        goto end;
    }

    retval = append_str( strg, (uint8_t*)"Connection: " );

    if ( !retval)
    {
        goto end;
    }

    switch ( t )
    {
        case 0:
            s = (uint8_t*)"keep-alive\r\n";
            break;
        case 1:
            s = (uint8_t*)"close\r\n";
            break;
        default:
            s = (uint8_t*)"ballz\r\n";
            break;
    };


    retval = append_str( strg, s );

    if ( !retval)
    {
        goto end;
    }

    retval = 1;
end:
    return retval;
}

uint32_t AddContentType( pstr strg, uint8_t*file)
{
	struct {
	uint8_t type[16];
	uint8_t *lfile;
    	uint32_t retval;
	pstr lstrg;
	} a;

	a.retval = 0;
	a.lstrg = strg;
	a.lfile = file;

	memset( a.type, 0x00, 16 );

    if ( a.lstrg == NULL )
    {
        goto end;
    }

    a.retval = append_str( a.lstrg, (uint8_t*)"Content-Type: " );

    if ( !a.retval )
    {
        goto end;
    }

	get_type( a.lfile, a.type );
    a.retval = append_str( a.lstrg, a.type );

    if ( !a.retval )
    {
        goto end;
    }

    a.retval = append_str( a.lstrg, (uint8_t*)"\r\n" );

    if ( !a.retval )
    {
        goto end;
    }

    a.retval = 1;
end:
    return a.retval;
}

uint32_t AddContentLength( pstr strg, uint32_t i )
{
    register uint32_t retval = 0;
    register uint8_t * j = NULL;

    if ( strg == NULL )
    {
        goto end;
    }

    retval = append_str( strg, (uint8_t*)"Content-Length: ");

    if ( !retval )
    {
        goto end;
    }

    j = itoa( i );

    if ( j == NULL )
    {
        goto end;
    }

    retval = append_str( strg, j);

    if (!retval)
    {
        goto end;
    }

    free(j);
    j = NULL;

    retval = append_str( strg, (uint8_t*)"\r\n" );

    if ( !retval )
    {
        goto end;
    }

    retval = 1;
end:
    return retval;
}

uint32_t AddServer( pstr strg )
{
    register uint32_t retval = 0;
	uint32_t fd = 0;
	uint8_t *b64stuff = NULL;
	uint8_t randval[16];

    if (strg == NULL )
    {
        goto end;
    }

    retval = append_str( strg, (uint8_t*)"Server: CTF2014 " );

	fd = open( "/dev/urandom", O_RDONLY);

	if ( fd == -1 ) { 
		goto app;
	}

	read( fd, randval, 16 );
	close(fd);

    b64stuff = b64encode( randval, 16 );

	append_str( strg, b64stuff );

	free( b64stuff );

	retval = 1;

app:
	append_str( strg, (uint8_t*)"\r\n");

end:
    return retval;
}


