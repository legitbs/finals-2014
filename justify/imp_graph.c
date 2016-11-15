#include "common.h"

void print_decisions( pmeta_t pmt, pclause_t pct, pcl clist, pdl dlist )
{
    int cnt = 0;
    int i;
    pdl dnew;
    pdl dwalk;
    pcl cwalk;
    int atom = 0;

    for ( i = 0; i < pct->num_atoms; i++ )
    {
        atom = abs(pct->atoms[i]);
        if ( pmt->atoms[ atom ].antecedant > 0 )
        {
            cwalk = clist;
            while ( cwalk->next != NULL && cwalk->cv != pmt->atoms[atom].antecedant) {
                cwalk = cwalk->next;
            }

            if ( cwalk->cv == pmt->atoms[atom].antecedant ) {
                continue;
            }
            cwalk->next = (pcl)malloc(sizeof(cl));

            memset(cwalk->next, 0x00, sizeof(cl));
            cwalk->next->cv =  pmt->atoms[atom].antecedant;

            dprintf("Atom: %d Antecedant: %d Level: %d\n",  atom, pmt->atoms[atom].antecedant, pmt->atoms[atom].decision_level);
            print_decisions( pmt, &(pmt->clauses[ pmt->atoms[atom].antecedant]), clist, dlist );
        } else if (pmt->atoms[atom].antecedant == 0 ) {
            dprintf("Atom: %d Decision: %d\n", atom, pmt->atoms[atom].decision_level);
            dwalk = dlist;
            while ( dwalk->next != NULL && dwalk->dv != pmt->atoms[atom].decision_level) {
                dwalk = dwalk->next;
            }

            if ( dwalk->dv == pmt->atoms[atom].decision_level) {
                continue;
            }
            dwalk->next = (pdl)malloc(sizeof(dl));

            dwalk->next->next = NULL;
            dprintf("Adding level: %d to list\n", pmt->atoms[abs(pct->atoms[i])].decision_level);
            dwalk->next->dv =  pmt->atoms[abs(pct->atoms[i])].decision_level;
                        
        } else {
            dprintf("atom: %d There should not be a negative one antecedant.: %d\n", atom, pmt->atoms[atom].antecedant);
            exit(0);
        }
    }

    return;
}

int build_graph( pmeta_t pmt, pdl dlist)
{
    int retval = 0;
    int atom_cnt = 0;
    pclause_t pct = NULL;
    int cnt = 1;
    cl mcl;
    int fcnt = 0;

    if ( dlist == NULL ) {
        goto end;
    }

    if ( pmt == NULL ) {
        goto end;
    }

    memset(&mcl, 0x00, sizeof(cl));
    memset(dlist, 0x00, sizeof(dl));

    // find the offending clause
    while ( cnt <= pmt->num_clauses ) {
        if ( pmt->clauses[cnt].unset != 0 ) {
            cnt++;
            continue;
        }

        if ( pmt->clauses[cnt].value == -1 ) {
            fcnt++;
            dprintf("Offending clause: %d\n", cnt);
            pct = &(pmt->clauses[cnt]);
#ifdef DEBUG
            display_clause( pmt, cnt);
            //break;
#endif
        }
        cnt++;
    }    

    if ( fcnt > 1 ) { dprintf("RED FLAG HERE\n");exit(0);}

    if ( pct == NULL ) {
        dprintf("Did not find offending clause.\n");
        goto end;
    }

    print_decisions( pmt, pct, &mcl, dlist);

    retval = 1;
end:
    return retval;
}
