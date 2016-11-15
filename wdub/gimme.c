#include "myscript.c"

uint32_t handle_gimme( pvar_array pva, uint8_t*newvar, uint8_t*argone)
{
    uint32_t retval = 0xffffffff;
    uint32_t size = 0;
    uint32_t counter = 0;
    pvar pv = NULL;
    parrtype part = NULL;

    if ( !pva || !newvar || !argone ) {
        goto end;
    }

	// If the variable exists already I will not create another
    if ( varExists( pva, newvar ) ) {
        goto end;
    }

	// Cheap isnum
    while ( argone[counter] ) {
        if ( argone[counter] < '0' && argone[counter] >'9'){
            goto end;
        }
        counter++;
    }

    size = atoi( (const char *)argone );

	// Allocate the variable management structure.
    pv = malloc( sizeof(var) );

    if ( pv == NULL ) {
        goto end;
    }

    memset( pv, 0x00, sizeof(var));

    pv->varname = malloc( strlen((const char *)newvar) + 1 );

    if (pv->varname == NULL ) {
        free(pv);
        pv = NULL;
        goto end;
    }

    memset(pv->varname, 0x00, strlen((const char *)newvar) + 1);
    memcpy( pv->varname, newvar, strlen((const char *)newvar));

	// Allocate an array structure with the correct buffer size hopefully
    part = malloc( sizeof( arrtype ) + size );

	if ( part == NULL ) {
		free(pv->varname);
		pv->varname = NULL;
		free(pv);
		pv = NULL;
		goto end;
	}

	pv->vardata = (uint8_t*)part;

    part->type = _array;
    part->len = size;
    pva->vars[ pva->numvars ] = pv;
    pva->numvars++;

    retval = 0;
end:
    return retval;
}

