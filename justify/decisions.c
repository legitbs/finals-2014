#include "common.h"

int add_decision_level( pnds ds, uint32_t atom, int32_t val )
{
    register int retval = 0;
    register uint32_t *tdl = NULL;
    register int8_t *tdlv = NULL;

    if ( ds == NULL ) {
        goto end;
    }

    if ( ds->decision_level + 2 > ds->max_len ) {
        dprintf("[META] Expanding decision stack to %d\n", ds->max_len*2);

        ds->max_len *= 2;
        tdl = (uint32_t*)malloc( ds->max_len * sizeof(uint32_t));

        if ( tdl == NULL ) {
            goto end;
        }

        tdlv = (int8_t*)malloc( ds->max_len * sizeof(int8_t));

        if ( tdlv == NULL ) {
            free(tdl);
            goto end;
        }

        memset( tdl, 0x00, ds->max_len * sizeof(uint32_t));
        memset( tdlv, 0x00, ds->max_len * sizeof(int8_t));

        memcpy( tdl, ds->decision_list, ds->decision_level * sizeof(uint32_t));
        memcpy( tdlv, ds->decision_list_values, ds->decision_level * sizeof(int8_t));

        free( ds->decision_list );
        free( ds->decision_list_values );

        ds->decision_list = tdl;
        ds->decision_list_values = tdlv;
    }

    ds->decision_level++;
    ds->decision_list[ ds->decision_level ] = atom;
    ds->decision_list_values[ ds->decision_level ] = val;

    retval = ds->decision_level;

end:
    return retval;
}

int dl_tried_both( pnds ds )
{
    if ( ds == NULL ) {
        exit(0);
    }

    return abs( ds->decision_list_values[ ds->decision_level ]) >> 1;
}
