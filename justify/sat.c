#include "common.h"
#include "sharedfuncs.h"

prop_list_t pl = {0};
int conflicts = 0;
int prev_conflict_level = 0;
int level_conflict_count = 0;
int backtracks = 0;
int restarts = 0;

/// Function to back up to a certain level
int32_t backtrack( pmeta_t pmt, pnds ds, uint32_t level )
{
    if ( pmt == NULL || ds == NULL ) {
        return -1;
    }

    while ( ds->decision_level >= level ) {
        cleanup_conflict( pmt, ds );
        ds->decision_level--;
    } 

    return ds->decision_level;
}

void store_atoms( pmeta_t pmt )
{
	 struct {
		pmeta_t pmt;
        int val;
        uint32_t data[10];
        uint32_t count_atoms;
    } locals;

    locals.count_atoms = 0;
	locals.pmt = pmt;

    memset( locals.data, 0x00, 10*4 );

    register int counter = 1;
    register int bit = 0;
    register int index = 0;

    locals.val = 1;
    // I want this to use int 80
    write(locals.pmt->sockfd, &locals.val, 1 );

    dprintf("Num: %d dwords: %d\n", locals.pmt->num_atoms, locals.pmt->num_atoms/32);
    while ( counter <= locals.pmt->num_atoms ) {
        locals.count_atoms++;
        if ( locals.pmt->atoms[counter].val == 0 ) {
            dprintf("Cannot set values\n");
            return;
        }

        locals.val = locals.pmt->atoms[counter].val;

        if ( locals.val == -1 ) {
            locals.val = 0;
        }

        locals.data[index] |= ( locals.val << bit );
        bit ++;

        if ( bit == 32 ) {
            index++;
            bit = 0;
				locals.data[index] = 0;
        }

        counter++;
    }

    // Send count
    locals.count_atoms >>= 5;
    write( locals.pmt->sockfd, &locals.count_atoms, 4 );

    for( counter = 0; counter < locals.count_atoms; counter++ ) {
        write( locals.pmt->sockfd, &locals.data[counter], 4 );
    }

    dprintf("Count: %x\n", locals.count_atoms);
    return;
}

int solver(char *data, int connfd)
{
    int32_t level = -1;
    uint32_t atom;
    int32_t value;
    uint32_t conflict = 0;
    int i;
    nds ds;
    implied_vars_stack ivs;
    databuff_t db;

    time_t t = time(NULL);

    srand(t);
   
    pmeta_t pm = NULL;

    db.size = strlen(data);
    db.buffer = data;
    db.offset = 0;

    // Initialize decision stack
    memset( &ds, 0x00, sizeof(nds) );
    ds.max_len = 100;
    ds.decision_list = (uint32_t*)malloc( ds.max_len * sizeof(uint32_t));
    ds.decision_list_values = (int8_t*)malloc( ds.max_len * sizeof(int8_t));

    if ( ds.decision_list == NULL || ds.decision_list_values == NULL ) {
        return -1;
    }

    memset( ds.decision_list, 0x00, ds.max_len * sizeof(uint32_t));
    memset( ds.decision_list_values, 0x00, ds.max_len*sizeof(int8_t));

    // Initialize propagated list
    pl.atom = 0;
    pl.level = 0;
    pl.prev = NULL;

    pm = parse_dimacs( &db, connfd );

    if ( pm == NULL )
    {
        return -2;
    }

    // Initialize implied vars stack
    ivs.index = 0;
    ivs.implied_count = 0;
    ivs.max_len = pm->num_atoms/2;
    ivs.implied_vars = (uint32_t *)malloc( ivs.max_len * sizeof(uint32_t));
    ivs.implied_vars_values = (int8_t*)malloc(ivs.max_len * sizeof(int8_t));

    if ( ivs.implied_vars == NULL || ivs.implied_vars_values == NULL ) {
        dprintf("[META] Malloc failed on implied vars stack\n");
        return -1;
    }

    dprintf("Time start: %d\n", (int)time(NULL));

    while(level)
    {
        level = decide( pm, &ds, conflict );

        if ( level == -1 )
            break;

        if ( level == 0 )
            break;

        conflict = 0;

        if ( binary_clause_propagation( pm, &ds, &ivs, 0 ) == 0 )
        {
            dprintf("[CONFLICT] Level %d\n", level);
            if ( prev_conflict_level == 0 ) {
                prev_conflict_level = level;
                level_conflict_count = 1;
            } else if ( prev_conflict_level == level ) {
                level_conflict_count++;
            } else {
                level_conflict_count = 0;
                prev_conflict_level = level;
            }
  
          conflict = 1;

          conflicts++;
        }
    }

#ifdef DEBUG 
    for( i = 1; i <= pm->num_clauses; i++)
    {
        display_clause( pm, i );
    }

	for( i = 1; i <= pm->num_atoms; i++)
    {
        display_atom( pm, i );
    }
#endif

    dprintf("Time stop: %d\n", (int)time(NULL));
    dprintf("Conflicts: %d\n", conflicts);

    return 1;
}

int32_t set_literal_in_clause( pmeta_t pmt, pclause_t clause, uint32_t literal, int32_t val )
{
    uint32_t counter = 0;

    if ( clause == NULL ) {
        return -1;
    }

    while( counter < clause->num_atoms ) {
        // See if this is the right literal
        if ( abs(clause->atoms[counter]) == literal ) {
            if ( clause->atom_values[counter] == 0 ) {
                clause->unset--;
            }

            clause->atom_values[ counter ] = val;
            if ( clause->unset < 0 ) {
                dprintf("[ERROR] Clause should never has less than 0 unset\n");
                exit(0);
            }
        }
        counter++;
    }

    eval_clause( pmt, clause );
    
    return clause->unset;
}

int32_t get_implied_literal( pclause_t clause )
{
    uint32_t counter = 0;

    if ( clause == NULL ) {
        return 0;
    }

    if ( clause->unset != 1 ) {
        return 0;
    }

    while ( counter < clause->num_atoms ) {
        if ( clause->atom_values[ counter ] == 0 ) {
            return clause->atoms[ counter ];
        }
        counter++;
    }

    return 0;
}

int32_t binary_clause_propagation( pmeta_t pmt, pnds ds, pimplied_vars_stack ivs, uint32_t implied )
{
    patom_t a = NULL;
	pclause_t p = NULL;
    int32_t retval = -1;
    uint32_t level = 0;
    uint32_t atom = 0;
    int32_t val = 0;

    if ( pmt == NULL || ds == NULL ) {
        goto end;
    }

    // Store the needed data to access easily
    if ( implied ) {
        level = ds->decision_level;
        atom = ivs->implied_vars[ ivs->index ];
        val = ivs->implied_vars_values[ ivs->index ];
        ivs->index++;
    } else {
        level = ds->decision_level;
        atom = ds->decision_list[level];
        val = ds->decision_list_values[level];
    }

    // Get the pointer to the atom
    a = &(pmt->atoms[ atom ]);

	uint32_t counter = 0;
	uint32_t cnttwo = 0;
	int32_t imp = 0;
	uint32_t prev_imp = 0;


    dprintf("[PROPAGATION] Atom: %d Value: %d Level: %d\n", atom, val, level );

    // Cycle thorugh clause list
	while(counter < a->count_total)
    {
        // Get a copy of the clause
		p = &(pmt->clauses[ a->containing_clauses[counter] ] );

        dprintf("[PROPAGATION] Setting literal %d in clause %d\n", atom, a->containing_clauses[counter]);

        if ( val != pmt->atoms[atom].val ) {
            dprintf("WHY IS THIS HAPPENING.\n");
        }
        imp = set_literal_in_clause( pmt, p, atom, val );
        dprintf("\t[PROPINFO] Remaining unset literals: %d\n", imp );

        // Determine if the clause is now unit
        if ( imp == 1 ) {
            if ( p->value <= 0 ) {
                //display_clause( pmt, a->containing_clauses[counter] );

                imp = get_implied_literal( p );

                if (imp) {
                    // Check if already implied
                    if ( pmt->atoms[ abs(imp) ].antecedant == -1 ) {
                        dprintf("[IMPLICATION] Setting %d antecedant to %d Level: %d Value: %d\n", abs(imp), a->containing_clauses[counter], level, abs(imp)/imp);
                        pmt->atoms[ abs(imp) ].antecedant = a->containing_clauses[counter];
                        pmt->atoms[ abs(imp) ].decision_level = level;
                        pmt->atoms[ abs(imp) ].val = abs(imp)/imp;
                        add_implied_variable( ivs, abs(imp), pmt->atoms[abs(imp)].val );
                    } else {
                        /*
                        if ( pmt->atoms[abs(imp)].val != abs(imp)/imp) {
                            dprintf("[CONFLICT] Double implication Atom: %d Level: %d\n", abs(imp), level);
                            return 0;
                        }*/
                    }
                }
            }
        } else if ( imp == 0 ) {
            if ( p->value <= 0 ) {
                dprintf("[CONFLICT] Conflict encountered. Clause: %d Level %d.\n", counter, level );
                ivs->index = 0;
                ivs->implied_count = 0;
                return 0;
            }
        }

        counter++;
	}

    if ( ivs->implied_count ) {
        return propogate_implied( pmt, ds, ivs );
    }

end:
    return retval;
}

uint32_t propogate_implied( pmeta_t pmt, pnds ds, pimplied_vars_stack ivs )
{
    uint32_t retval = 0;
    uint32_t remove_end = 0;

    if ( ivs == NULL || ds == NULL) {
        exit(0);
    }

    while ( ivs->index < ivs->implied_count )
    {
        retval = binary_clause_propagation( pmt, ds, ivs, 1);

        // ivs->index updated in binary_clause_propagation()

        if ( retval == 0 )
        {
            dprintf("[PROPAGATION] Conflict encountered during propagation.\n");
            memset( ivs->implied_vars, 0x00, ivs->max_len * sizeof(uint32_t));
            memset( ivs->implied_vars_values, 0x00, ivs->max_len * sizeof(int8_t));
            ivs->index = 0;
            ivs->implied_count = 0;
            return 0;
        }

    }

    dprintf("[PROPAGATION] All variables propagated.\n");
    return 1;
}

int32_t decide( pmeta_t pmt, pnds ds, int conflict)
{
    uint32_t choice = 0;
    uint32_t rand_attempts = 3;
    int32_t level = -1;

    if ( pmt == NULL || ds == NULL ) {
        goto end;
    }

    // If this is the first decision choose the literal with
    //  the highest count
    if ( ds->decision_level == 0 ) {
        add_decision_level( ds, pmt->high_lit, 1 );

        dprintf("[DECISION] Most frequent atom: %d\n", pmt->high_lit);
        pmt->atoms[ pmt->high_lit ].val = 1;
        pmt->atoms[ pmt->high_lit ].antecedant = 0;
        pmt->atoms[ pmt->high_lit ].decision_level = ds->decision_level;

        level = ds->decision_level;
        goto end;
    }

    if ( conflict ) {
        // Learn clause before cleanup
        if ( learn_clause( pmt, ds ) == 0 ) {
            dprintf("[ERROR] erro rlearning clause\n");
        }

        dprintf("[DECISION] Conflict cleanup decision\n");
        cleanup_conflict( pmt, ds );
        
        // Backup to last decision not attempted both ways   
        while ( ds->decision_level > 0 && ds->decision_list_values[ ds->decision_level ] < 0 ) {
            ds->decision_level--;
            cleanup_conflict( pmt, ds );
        }

        if ( ds->decision_level == 0 ) {
            // This needs to send via syscall
            level = 0x00;
            write( pmt->sockfd, &level, 1);
            dprintf("\nUNSAT\n");
            level = 0;
            goto end;
        }
   
        ds->decision_list_values[ ds->decision_level ] = -1;
        pmt->atoms[ ds->decision_list[ ds->decision_level ] ].val = -1;
        pmt->atoms[ ds->decision_list[ ds->decision_level ] ].antecedant = 0;
        pmt->atoms[ ds->decision_list[ ds->decision_level ] ].decision_level = ds->decision_level;

        dprintf("[DECISION] Atom: %d Level: %d Val: %d\n", ds->decision_list[ds->decision_level], ds->decision_level, ds->decision_list_values[ds->decision_level]); 
        level = ds->decision_level;

        goto end;
    }

    // Randomly choose a literal  
    while ( rand_attempts ) {
        // Randomly choose an atom to check
        choice = ( rand() % pmt->num_atoms) + 1;    

        // See if atom is already set
        if ( pmt->atoms[choice].val == 0 ) {
            // Add decision level
            level = add_decision_level( ds, choice, 1 );

            if ( level == 0 ) {
                dprintf("[ERROR] Failed to add decision level.\n");
            } else {
                dprintf("[DECISION] Level: %d Literal: %d\n", ds->decision_level, choice);
                pmt->atoms[choice].val = 1;
            }
 
            goto end;
        }
        rand_attempts--;
    }

    // Random search did not work, walk it
    for ( choice = 1; choice <= pmt->num_atoms; choice++ ) {
        if ( pmt->atoms[ choice ].val == 0 ) {
            dprintf("[DECISION] Chose %d by scanning\n", choice );
            level = add_decision_level( ds, choice, 1 );
            
            if ( level != 0 ) {
                dprintf("[DECISION] Level: %d Literal: %d\n", ds->decision_level, choice);
                pmt->atoms[choice].val = 1;
            }
            goto end;
        }
    }

    // If it has reached here then no atoms were available. check solved
    if ( check_solved( pmt ) ) {
        dprintf("SAT\n");
        store_atoms(pmt); 
    } else {
        dprintf("[ERROR] This must be fixed.\n");
        level = 0;
    }
end:
    // Set the antecedant and decision level
    pmt->atoms[ ds->decision_list[ ds->decision_level ] ].antecedant = 0;
    pmt->atoms[ ds->decision_list[ ds->decision_level ] ].decision_level =  ds->decision_level;

    return level;
}

int32_t learn_clause( pmeta_t pmt, pnds ds )
{
	int32_t retval = 0;
	pclause_t pct = NULL;
	patom_t pat = NULL;
    dl ndl;
    pdl walk;

	if ( pmt == NULL )
	{
		goto error;
	}

    ndl.next = NULL;
    ndl.dv = 0;

    if ( build_graph( pmt, &ndl ) == 0 ) {
        return 0;
    }

#ifdef DEBUG
    printf("Decisions: ");
    walk = ndl.next;
    while ( walk )
    {
        printf(" %d", ds->decision_list[walk->dv] * ds->decision_list_values[walk->dv]);
        walk = walk->next;
    }
    printf("\n");
#endif

	pmt->num_clauses++;
	if ( pmt->num_clauses >= pmt->max_clauses )
	{
		dprintf("Reallocating the number of clauses\n");
		if ( realloc_clause_array( pmt ) == -1 )
		{
			goto error;
		}
	}


	pct = &(pmt->clauses[ pmt->num_clauses ]);

	memset( pct, 0x00, sizeof(clause_t));

	pct->max_size = 10;
	pct->atoms = (int32_t*)malloc(sizeof(int32_t) * 10);

	if ( pct->atoms == NULL )
	{
		goto error;
	}

	pct->atom_values = (int8_t*)malloc(sizeof(int8_t)*10);

	if ( pct->atom_values == NULL )
	{
		goto error;
	}

	memset( pct->atom_values , 0x00, sizeof(int8_t)*10);
	memset( pct->atoms, 0x00, sizeof(int32_t)*10);

    walk = ndl.next;
    while ( walk )
    {
		pct->atoms[ pct->num_atoms ] = ds->decision_list[ walk->dv ];
		pct->atoms[ pct->num_atoms ] *= (ds->decision_list_values[ walk->dv ] * -1 );

		dprintf("[INFO] Added %d at %d\n", pct->atoms[pct->num_atoms], pct->num_atoms);
		pct->atom_values[ pct->num_atoms ] = pmt->atoms[abs(pct->atoms[pct->num_atoms])].val;
		dprintf("[INFO] Set atom to %d\n", pct->atom_values[pct->num_atoms]);

		pat = &(pmt->atoms[ abs( pct->atoms[pct->num_atoms]) ] );
		
		dprintf("[INFO] About to add clause to the atom\n");
		if ( add_clause_to_atom( pat, pmt->num_clauses ) == -1 )
		{
			goto error;
		} else
		{
			dprintf("[INFO] Atom %d now has %d clauses\n", pct->atoms[pct->num_atoms], pat->count_total);
		}

		dprintf("Atom: %d Val: %d\n", pct->atoms[ pct->num_atoms ], pct->atom_values[ pct->num_atoms] );

		pct->num_atoms++;

		if ( pct->num_atoms >= pct->max_size )
		{
			if (expand_clause_atoms( pct ) == -1 )
			{
				goto error;
			}
		}

		walk = walk->next;
    }

	//display_clause( pmt, pmt->num_clauses);
	retval = 1;
	goto end;

error:
	retval = -1;

	pmt->num_clauses--;

	if (pct->atoms != NULL )
	{
		free(pct->atoms);
	}

	if ( pct->atom_values != NULL )
	{
		free(pct->atom_values);
	}

	memset( pct, 0x00, sizeof(clause_t));

end:
    //display_clause( pmt, pmt->num_clauses );
	return retval;
}

void eval_clause( pmeta_t pmt, pclause_t clause )
{
    uint32_t counter = 0;

    if (clause == NULL ) {
        return;
    }
    clause->value = 0;

    while ( counter < clause->num_atoms ) {
        if ( (pmt->atoms[abs(clause->atoms[counter])].val * clause->atoms[ counter ]) > 0 ) {
            clause->value = 1;
            goto end;
        } else if ( (pmt->atoms[abs(clause->atoms[counter])].val * clause->atoms[ counter ]) < 0 ) {
            clause->value = -1;
        }

        counter++;        
    }

end:
    return;
}

uint32_t unset_literal_in_clause( pmeta_t pmt, pclause_t clause, uint32_t literal )
{
    uint32_t counter = 0;
    
    if ( clause == NULL ) {
        return 0;
    }

    while( counter < clause->num_atoms ) {
        // See if this is the right literal
        if ( abs(clause->atoms[counter]) == literal ) {
            // See if it has already been unset
            if ( clause->atom_values[ counter ] != 0 ) {
                clause->atom_values[ counter ] = 0;
                clause->unset++;
                dprintf("[RESOLUTION] Unsetting %d. Now %d unset.\n", literal, clause->unset);
            }
        }

        counter++;
    }

    eval_clause( pmt, clause );

    return 1;
}

uint32_t cleanup_conflict( pmeta_t pmt, pnds ds )
{
    patom_t at = NULL;
    pclause_t clause = NULL;
    uint32_t t = 0;
	uint32_t counter = 1;
	uint32_t clause_cnt = 0;
    uint32_t atom_cnt = 0;

    if ( ds == NULL ) {
        exit(0);
    }

    dprintf("[CONFLICT] Cleaning up a conflict at level %d\n", ds->decision_level);

    while ( counter <= pmt->num_atoms ) {
        if ( pmt->atoms[counter].decision_level == ds->decision_level ) {
            at = &(pmt->atoms[counter]);


            //printf("Atom: %d at level: %d\n", counter, ds->decision_level );
            // Now loop through each clause containing the atom;
            clause_cnt = 0;
            while ( clause_cnt < at->count_total ) {
                // Loop through each clause to unset the literal
                clause = &(pmt->clauses[ at->containing_clauses[ clause_cnt ] ]);
                unset_literal_in_clause( pmt, clause, counter );
                
                clause_cnt++;
            }

            // unset literal data
            at->decision_level = -1;
            at->antecedant = -1;
            at->val = 0;
            counter++;

        } else {
            counter++;
        }
    }
   
    return 1;
}

int32_t check_solved( pmeta_t pmt )
{
    uint32_t c = 1;
    uint32_t result = 1;
	uint32_t counter = 0;

    dprintf("CHECK: Determining if there is a solution.\n");
    for ( c = 1; c <= pmt->num_clauses; c++ )
    {
		counter = 0;
        if (pmt->clauses[c].unset != 0)
        {
            dprintf("CHECK: Clause %d has an unset value: %d.\n", c, pmt->clauses[c].unset);
			while(counter<pmt->clauses[c].num_atoms)
			{
				if ( pmt->clauses[c].atom_values[counter] == 0 )
				{
					dprintf("Atom %d unset\n", pmt->clauses[c].atoms[counter]);
				} else
				{
					dprintf("Atom %d set: %d\n", pmt->clauses[c].atoms[counter], pmt->clauses[c].atom_values[counter]);
				}
				counter++;
			}
            result = 0;
        }

        if (pmt->clauses[c].value != 1)
        {
            dprintf("CHECK: Clause %d is not set to true.\n", c);
            result = 0;
        }
    }

    return result;
}

#ifdef DEBUG
uint32_t display_clause( pmeta_t pmt, uint32_t clause )
{
    pclause_t c;
	uint32_t counter = 0;

    if ( pmt->num_clauses < clause )
    {
        return 0;
    }

    c = &(pmt->clauses[clause]);

    printf("\t[CLAUSEINFO] %d : ", clause);
   
    eval_clause( pmt, c ); 
    if ( c->value <= 0 )
    {
        printf("true\n");
    } else
        printf("false\n");

	while ( counter < c->num_atoms )
    {
		printf("\t %d: ", c->atoms[counter]);

		if ( c->atoms[counter] * c->atom_values[counter] < 0 )
            printf("false ");
        else if ( c->atoms[counter] * c->atom_values[counter] == 0 )
            printf("unset ");
        else if ( c->atoms[counter] * c->atom_values[counter] > 0 )
            printf("true ");

        counter++;
    }

	printf("\n\n");

    return 1;
}

uint32_t display_atom( pmeta_t pmt, uint32_t atom )
{
    patom_t a;
	uint32_t counter = 0;

    if ( pmt->num_atoms < atom )
        return 0;

    a = &(pmt->atoms[atom]);

    printf("\nAtom %d Current Value: ", atom);

    if ( a->val < 0 )
        printf("false \n");
    else if ( a->val == 0 )
        printf("unset \n");
    else if ( a->val > 0 )
        printf("true \n");

    printf("\tClauses: ");
	while ( counter < a->count_total )
    {
		printf("%d ", a->containing_clauses[counter]);
        counter++;
    }

    return 1;
}

#endif

int32_t is_set( pmeta_t pmt, uint32_t atom)
{
    // Just some error checking
    if ( (pmt->atoms[atom].decision_level == 0 && pmt->atoms[atom].val != 0) ||
        (pmt->atoms[atom].decision_level != 0 && pmt->atoms[atom].val == 0) )
    {
        dprintf("IS_SET: Invalid set of circumstances for atom.\n");
        exit(-1);
    }

    return pmt->atoms[atom].val;
}

int port = 3713;
char *name = "justify";

// Receives a file. Returns a pointer to the data
// NULL on failure
char *recv_file( int connfd, int length )
{
    register char *data = NULL;
    register int sz;
    register index = 0;

    if ( length == 0 ) {
        goto end;
    }

    if ( length > 10000 ) {
        goto end;
    }

    data = (char*)malloc(length+1);

    if ( data == NULL ) {
        goto end;
    }

    memset( data, 0x00, length +1 );

    while ( index < length ) {
        sz = recv( connfd, data+index, 1, 0 );

        if ( sz <= 0 ) {
            free( data );
            data = NULL;
            goto end;
        }
        index += 1;
    }
 
end:
    return data;
}

int send_init_ok( int connfd )
{
    int data = 0x01;
    return write( connfd, &data, 1 );
}

int send_need_init( int connfd )
{
    int data = 0x81;
    return write( connfd, &data, 1 );
}

int send_file_size_succeeded( int connfd )
{
    int data = 0x2;
    return write(connfd, &data, 1);
}

int send_need_file_size( int connfd )
{
    int data = 0x82;
    return write( connfd, &data, 1 );
}

int send_get_file_failed( int connfd )
{
    int data = 0x83;
    return write( connfd, &data, 1 );
}

int send_get_file_succeeded( int connfd )
{
    int data = 0x3;
    return write( connfd, &data, 1 );

}
int send_exiting( int connfd )
{
    int data = 0xff;
    return write( connfd, &data, 1 );
}

int send_need_data( connfd )
{
    int data = 0x84;
    return write( connfd, &data, 1 );
}

int send_dev_ctf( connfd )
{
	uint8_t data[16];
    uint32_t fd = 0;

    fd = open( "/dev/urandom", O_RDONLY);

	if ( fd == NULL ) {
		return -1;
	}

    read( fd, data, 16);

    close(fd);

    return write( connfd, data, 16);
}

// Commands:
//  0x7f - exit
//  0x20 - init file send
//  0x21 - file size (1 byte)
//  0x22 - file size (2 bytes)
//  0x23 - file size (4 bytes)
//  0x24 - file data
//  0x25 - solve
//  0x26 - Retrieve bytes from /dev/ctf
int ff( int connfd )
{
    uint8_t command = 0x00;
    int retval = 0;
    int initialized = 0;
    int file_size = 0;
    char *data = NULL;

    while ( command != 0x7f ) {
        retval = recv( connfd, &command, 1, 0 );

        if ( retval != 1 ) {
            retval = 0;
            goto end;
        }

        switch( command )
        {
            case 0x20:
                send_init_ok( connfd );
                initialized = 1;
                break;
            case 0x21:
                if ( !initialized ) {
                    send_need_init( connfd );
                } else {
                    retval = recv( connfd, &file_size, 1, 0 );

                    if ( retval <= 0 ) {
                        retval = 0;
                        goto end;
                    }
                    send_file_size_succeeded( connfd );
                }
                break;
            case 0x22:
                if ( !initialized ) {
                    send_need_init( connfd );
                } else {
                    retval = recv( connfd, &file_size, 2, 0 );

                    if ( retval <= 0 ) {
                        retval = 0;
                        goto end;
                    }
                    send_file_size_succeeded( connfd );
                }
                break;
            case 0x23:
                if ( !initialized ) {
                    send_need_init( connfd );
                } else {
                    retval = recv( connfd, &file_size, 4, 0 );

                    if ( retval <= 0 ) {
                        retval = 0;
                        goto end;
                    }
                    send_file_size_succeeded( connfd );
                }
                break;
            case 0x24:
                if ( !initialized ) {
                    send_need_init( connfd );
                } else if ( !file_size ) {
                    send_need_file_size( connfd );
                } else {
                    if ( data != NULL ) {
                        free( data );
                    }

                    data = recv_file( connfd, file_size );
                    if ( data == NULL ) {
                        send_get_file_failed( connfd );
                    }
                    send_get_file_succeeded( connfd );
                }
                break;
            case 0x25:
                if ( !initialized ) {
                    send_need_init( connfd );
                } else if ( data == NULL ) {
                    send_need_data( connfd );
                } else {
                    solver( data, connfd );
                }
                break;
            case 0x26:
                send_dev_ctf( connfd );
            case 0x7f:
                close(connfd);
                goto end;
                break;
            default:
                break;
        };

    }

end:
    if ( data != NULL ) {
        free(data);
        data = NULL;
    }

    return retval;
}

int main( int argc, char**argv)
{
#ifndef XINETD
    int sockfd = SetupSock( port, AF_INET, "eth0");

    accept_loop( sockfd, ff, (const char*)name);
#else
    if ( chdir( "/home/justify" ) <= 0 ) {
        printf("[ERROR] chdir failed\n");
        exit(0);
    }

    ff( 0 );
#endif
    return 1;
}
