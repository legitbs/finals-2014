#include "wdub.h"

uint32_t write_file( pstr strg, uint8_t *fn )
{
	register uint32_t retval = 0;
	register uint32_t fd = 0;

	if ( !strg || !fn ) {
		goto end;
	}

	if ( !strg->str ) {
		goto end;
	}

	fd = open( (const char *)fn, O_WRONLY, 0 );

	if ( fd == 0 ) {
		goto end;
	}

	if ( write( fd,	strg->str, strg->max ) == -1 ) {
		close( fd );
		goto end;
	}

	close(fd);
	retval = 1;
end:
	return retval;
}

uint32_t cut_data ( pstr strg, uint32_t start, uint32_t length)
{
    register uint32_t retval = 0;
    register uint8_t *t = NULL;

    if ( !strg ) {
        goto end;
    }

    if ( strg->str == NULL ) {
        goto end;
    }

    if ( start + length > strg->max ) {
        goto end;
    }

    t = malloc( strg->max - length );

    if ( t == NULL ) {
        goto end;
    }

    memcpy( t, strg->str, start );
    memcpy( t + start, strg->str + start + length, strg->max-(length+start));
    free( strg->str );
    strg->str = t;
    strg->max = strg->max - length;

    retval = 1;

end:
    return retval;
}

uint32_t overwrite_data( pstr strg, uint8_t *data, uint32_t start, uint32_t length )
{
	register uint32_t retval = 0;
	//register uint8_t *t = NULL;
	//uint32_t size = 0;

	if ( !strg || !data ) {
		goto end;
	}

	if ( !strg->str ) {
		goto end;
	}

	if ( start > strg->max ) {
		goto end;
	}

	if ( start + length > strg->max ) {
		goto end;
	}

	memcpy( strg->str + start, data, length );
	
	retval = 1;

end:
	return retval;
}

uint32_t insert_data( pstr strg, uint8_t *data, uint32_t start, uint32_t length)
{
    register uint32_t retval = 0;
    register uint8_t *t = NULL;

    if ( !strg || !data ) {
        goto end;
    }

    if ( strg->str == NULL ) {
        goto end;
    }

    if ( start > strg->max ) {
        goto end;
    }

    t = malloc( strg->max + length );
    
    if ( t == NULL ) {
        goto end;
    }

    memcpy( t, strg->str, start );
    memcpy( t + start, data, length );
    memcpy( t + start + length, strg->str + start, strg->max - start );

    free(strg->str);
    strg->str = t;
    strg->max = strg->max + length;

    retval = 1;
end:
    return retval;
}

uint32_t append_data( pstr strg, uint8_t *data, uint32_t length )
{
    register uint32_t retval = 0;
    register uint8_t *t = NULL;

    if ( strg == NULL )
    {
        goto end;
    }

    if ( data == NULL )
    {
        retval = 1;
        goto end;
    }

    if ( length == 0 )
    {
        retval = 1;
        goto end;
    }

    if ( strg->str == NULL )
    {
        strg->str = (uint8_t*)malloc(length+1);

        if ( strg->str == NULL )
        {
            retval = 0;
            goto end;
        }

        memset( strg->str, 0x00, length+1);
        memcpy( strg->str, data, length );
        retval = 1;
        goto end;
    }

    t = (uint8_t*)malloc(strg->max + length + 1);

    if ( t == NULL )
    {
        retval = 0;
        goto end;
    }

    memset( t, 0x00, strg->max + length + 1 );
    memcpy(t, strg->str, strg->max);
    memcpy(t + strg->max, data, length );
    free(strg->str);
    strg->str = t;
    strg->max = strg->max + length;

    retval = 1;
end:
    return retval;
}

pstr create_str( uint8_t*data )
{
    register pstr new_str = NULL;
    register uint32_t length = 0;

    if ( data == NULL )
    {
        goto end;
    }

    new_str = (pstr)malloc(sizeof(str));

    if ( new_str == NULL )
    {
        goto end;
    }

    while( data[length] != '\x00' )
    {
        length++;
    }

    new_str->str = (uint8_t*)malloc(length+1);

    if ( new_str->str == NULL )
    {
        free(new_str);
        new_str = NULL;
        goto end;
    }

    memset( new_str->str, 0x00, length+1);
    memcpy( new_str->str, data, length );
    new_str->offs = 0;
    new_str->max = length;

end:
    return new_str;
}

uint32_t append_str( pstr strg, uint8_t *newdata )
{
    register uint32_t retval = 0;
    register uint32_t length = 0;
    register uint8_t *t = NULL;

    if ( strg == NULL )
    {
        goto end;
    }

    if ( newdata == NULL )
    {
        retval = 1;
        goto end;
    }

    while ( newdata[length] != '\x00')
    {
        length++;
    }

    if ( strg->str == NULL )
    {
        strg->str = (uint8_t*)malloc(length+1);

        if ( strg->str == NULL )
        {
            retval = 0;
            goto end;
        }

        memset( strg->str, 0x00, length+1);
        memcpy( strg->str, newdata, length );
        retval = 1;
        goto end;
    }

    t = (uint8_t*)malloc(strg->max + length + 1);

    if ( t == NULL )
    {
        retval = 0;
        goto end;
    }

    memset( t, 0x00, strg->max + length + 1 );
    memcpy(t, strg->str, strg->max);
    memcpy(t + strg->max, newdata, length );
    free(strg->str);
    strg->str = t;
    strg->max = strg->max + length;

    retval = 1;
end:
    return retval;
}

void free_str( pstr str )
{
    if ( str == NULL )
    {
        return;
    }

    if ( str->str != NULL )
    {
        free( str->str );
        str->str = NULL;
    }

    free ( str );

    return;
}

uint32_t skip_line ( pstr s )
{
    uint32_t retval = 0;

    if ( s == NULL )
    {
        goto end;
    }

    while ( s->offs < s->max )
    {
        if ( s->str[s->offs] == '\r' )
        {
            s->str[s->offs++] = '\x00';

            if ( s->offs >= s->max )
            {
                break;
            }

            if ( s->str[s->offs] == '\n' )
            {
                break;
            }

            if ( s->str[s->offs] == '\n' )
            {
                s->str[s->offs++] = '\x00';
                break;
            } else
            {
                goto end;
            }
        } else
        {
            s->offs++;
        }
    }

    retval = s->offs;
end:
    return retval;
}

pstr init_str( pstr s, uint8_t *data, uint32_t max )
{
    if ( data == NULL ) {
        goto end;
    }

    if ( s == NULL ) {
        s = (pstr)malloc(sizeof(str));
    }

    if ( s == NULL ) {
        goto end;
    }

    memset(s, 0x00, sizeof(str));

    s->str = data;
    s->offs = 0;
    s->max = max;

end:
    return s;

}

