#include "itoa.h" 

uint32_t catoi( uint8_t *a )
{
	register uint32_t retval = 0;
	register uint32_t val = 0;
	uint32_t max = 0;
	uint32_t multi = 1;

	if ( a == NULL ) {
		goto end;
	}

	max = strlen( (const char *)a);

	while ( max ) {
		if ( !isdigit( a[max-1] ) ) {
			goto end;
		}

		val += (a[max-1] - 0x30 ) * multi;
		max--;
		multi *= 10;
	}

	retval = val;
end:
	return retval;
}

uint8_t *itoa( uint32_t i )
{
    register uint8_t *r = NULL;
    register uint32_t length = 0;
    register uint32_t t = i;
    register uint32_t z = 1;

    if ( i == 0 )
    {
        r = (uint8_t*)malloc(2);
        if ( r==NULL)
        {
            goto end;
        }

        r[0] = '0';
        r[1] = '\x00';
        goto end;
    }

    while ( t )
    {
        length++;
        z *= 10;
        t /= 10;
    }

    r = ( uint8_t*)malloc(length+1);

    if ( r== NULL )
    {
        goto end;
    }
    
    memset(r, 0x00, length+1);

    t = i;
    while ( length )
    {
        z = t % 10;
        r[length-1] = z + 0x30;
        t /= 10;
        length--;
    }    
end:
    return r;
}
