#include "common.h"

int add_implied_variable( pimplied_vars_stack ivs, uint32_t atom, int32_t val )
{
    register int retval = 0;
    register uint32_t *tiv = NULL;
    register int8_t *tivv = NULL;

    if ( ivs == NULL ) {
        goto end;
    }

    if ( ivs->implied_count + 2 > ivs->max_len ) {
        dprintf("[META] Expanding Implied stack to %d\n", ivs->max_len*2);

        ivs->max_len *= 2;
        tiv = (uint32_t*)malloc( ivs->max_len * sizeof(uint32_t));

        if ( tiv == NULL ) {
            goto end;
        }

        tivv = (int8_t*)malloc( ivs->max_len * sizeof(int8_t));

        if ( tivv == NULL ) {
            free(tiv);
            goto end;
        }

        memset( tiv, 0x00, ivs->max_len * sizeof(uint32_t));
        memset( tivv, 0x00, ivs->max_len * sizeof(int8_t));

        memcpy( tiv, ivs->implied_vars, ivs->implied_count * sizeof(uint32_t));
        memcpy( tivv, ivs->implied_vars_values, ivs->implied_count * sizeof(int8_t));
        free( ivs->implied_vars );
        free( ivs->implied_vars_values );

        ivs->implied_vars = tiv;
        ivs->implied_vars_values = tivv;
    }

    ivs->implied_vars[ ivs->implied_count ] = atom;
    ivs->implied_vars_values[ ivs->implied_count ] = val;
    ivs->implied_count++;

    retval = ivs->implied_count;

end:
    return retval;
}
