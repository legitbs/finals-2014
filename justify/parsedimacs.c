#include "common.h"

int32_t realloc_clause_array( pmeta_t pmt )
{
    register uint32_t retval = 0;
    register pclause_t new_clauses = NULL;
    register uint32_t new_max = 0;

    if ( pmt == NULL )
    {
        goto end;
    }

    if ( pmt->clauses == NULL )
    {
        new_max = 0x100;
    } else
    {
        new_max = pmt->max_clauses * 2;
    }

    new_clauses = (pclause_t)malloc( sizeof(clause_t)*new_max+1 );

    if ( new_clauses == NULL )
    {
        goto end;
    }

    memset( new_clauses, 0x00, sizeof(clause_t) * new_max+1 );
    
    // copy all of the old clauses
    memcpy( new_clauses, pmt->clauses, sizeof(clause_t) * pmt->num_clauses );
    
    free(pmt->clauses);
    pmt->clauses = new_clauses;
    pmt->max_clauses = new_max;
    
    retval = 1;
end:
    return retval;
}

pmeta_t parse_dimacs( pdatabuff_t data, int connfd )
{
    pmeta_t pmt = NULL;
    uint32_t counter = 0;
    int32_t atom_val = 0;
    uint32_t atom_name = 0;
    uint32_t offset = 0;
    uint32_t length = 0;
    uint8_t *buffer = NULL;

    pclause_t clause = NULL;
    patom_t atom = NULL;

    if ( data == NULL )
    {
      dprintf("[ERROR] parse_dimacs() Invalid argument.\n");
		goto end;
    }

    if ( data->buffer == NULL || data->size == 0 )
    {
      dprintf("[ERROR] parse_dimacs() No DIMACS data provided.\n");
		goto end;
    }

    pmt = (pmeta_t)malloc( sizeof(meta_t) );

    if ( pmt == NULL )
    {
        dprintf("[ERROR] parse_dimacs() Unable to allocate meta buffer\n");
		goto end;
    }

    memset( pmt, 0x00, sizeof(meta_t) );
    pmt->sockfd = connfd;

    length = data->size;
    buffer = data->buffer;

    while ( offset < length )
    {
        if ( buffer[ offset ] == 'c' )
        {
            dprintf("[INFO] Found comment at %d\n", offset);

        } else if ( buffer[ offset] == 'p' )
        {
            dprintf("[INFO] Found p\n");
            if ( memcmp( buffer + offset, "p cnf ", 6 ) == 0 )
            {
                offset += 6;
            } else
            {
               dprintf("[ERROR] parse_dimacs() Unknown format\n");
               free( pmt );
					pmt = NULL;
					goto end;
            }

            pmt->num_atoms = atoi( (const char *)(buffer + offset) );

            dprintf("[INFO] Number of atoms: %d\n", pmt->num_atoms);

            offset += seek_next_num( buffer + offset, length - offset );

            pmt->num_clauses = atoi( (const char *)(buffer + offset) );

            dprintf("[INFO] Number of clauses: %d\n", pmt->num_clauses );

            pmt->atoms = (patom_t)malloc( sizeof(atom_t) * ( pmt->num_atoms + 1 ) );

            // Allocate 10x the amount for learned clauses
            pmt->clauses = (pclause_t)malloc( sizeof(clause_t) * (pmt->num_clauses * 10 ) + 1 );

            pmt->max_clauses = pmt->num_clauses * 10;

            pmt->high_lit = 0;
            pmt->high_cnt = 0;

            if ( pmt->atoms == NULL || pmt->clauses == NULL )
            {
               dprintf("[ERROR] Failed to allocate buffer for atoms and clauses.\n");
				   free( pmt );
					pmt = NULL;	
					goto end;
            }

            memset( pmt->atoms, 0x00, sizeof(atom_t) * (pmt->num_atoms + 1) );
            memset( pmt->clauses, 0x00, sizeof(clause_t) * (pmt->max_clauses) + 1 );
        } else
        {
			// Clauses are 1 indexed as opposed to 0 indexed REMEMBER THIS
            counter++;

            // easier to work with this way.
            clause = &(pmt->clauses[counter]);

			// Initialize values
            clause->value = 0;
			clause->num_atoms = 0;

			// Allocate three atoms initially
			clause->atoms = (int32_t*)malloc( sizeof(int32_t) * 3 );
			
			if ( clause->atoms == NULL )
			{
				dprintf("[ERROR] Failed to allocate buffer for atoms.\n");
				free( pmt );
				pmt = NULL;
				goto end;
			}

			memset( clause->atoms, 0x00, sizeof(int32_t) * 3 );
			
			// Allocate the atom values
			clause->atom_values = (int8_t*)malloc( sizeof(int8_t) * 3 );
			
			if ( clause->atom_values == NULL )
			{
				dprintf("[ERROR] Failed to allocate buffer for atom values.\n");
				free( pmt );
				pmt = NULL;
				goto end;
			}

			memset( clause->atom_values, 0x00, sizeof(int8_t) * 3 );

			clause->max_size = 3;

            while ( buffer[ offset ] != '0' && offset < length )
            {
				if ( clause->num_atoms >= clause->max_size )
				{
					if ( expand_clause_atoms( clause ) == -1 )
					{
						free(pmt);
						pmt = NULL;
						goto end;
					}
				}

                atom_val = atoi((const char *)(buffer + offset));
                atom_name = abs(atom_val);

				atom = &(pmt->atoms[atom_name]);
                atom->antecedant = -1;

				if ( add_clause_to_atom( atom, counter ) == -1 )
				{
					dprintf("[ERROR] Failed to add clause to atom\n");
					free(pmt);
					pmt = NULL;
					goto end;
				}

				if (atom->count_total > pmt->high_cnt )
				{
					pmt->high_cnt = atom->count_total;
					pmt->high_lit = atom_name;
				}

                // Setup clause
				clause->atoms[ clause->num_atoms ] = atom_val;
				clause->atom_values[ clause->num_atoms ] = 0;

                clause->num_atoms++;
                clause->unset++;

				// Add the clause to the atom
                offset += seek_next_num( buffer + offset, length - offset );
            }
        }

        offset += seek_next_line( buffer + offset, length - offset );
    }


	// If there was an error it is possible to have a lot of memory
	//		blocks left allocated but since the process will close it
	//		shouldn't matter.
end: 
    return pmt;
}

int32_t add_clause_to_atom( patom_t pat, uint32_t clause )
{
	int32_t retval = 0;
	uint32_t counter = 0;

	if ( pat == NULL )
	{
		dprintf("[ERROR] Null in argument list\n");
		goto error;
	}

	while ( counter < pat->count_total )
	{
		if ( pat->containing_clauses[counter] == clause )
		{
			dprintf("[INFO] Atom already has this clause %d.\n", clause);
			goto end;
		}
		counter++;
	}

	if ( pat->count_total >= pat->max_size )
	{
		dprintf("[INFO] About to expand atom clauses\n");
		if ( expand_atom_clauses( pat ) == -1 )
		{
			dprintf("[ERROR] Failed to expand atom clauses.\n");
			goto error;
		} else
		{
			dprintf("[INFO] Clause expansion successful\n");
		}
	}

	pat->containing_clauses[ pat->count_total ] = clause;
	pat->count_total++;

	retval = pat->count_total;
	dprintf("[INFO] Atom has %d clauses with a max of %d\n",pat->count_total, pat->max_size);
	goto end;

error:
	retval = -1;
end:
	return retval;
}

int32_t expand_atom_clauses( patom_t pat )
{
	int32_t retval = 0;
	uint32_t new_max = 0;
	uint32_t *new_clauses = NULL;

	if (pat == NULL )
	{
		dprintf("[ERROR] NULL pointer in argument\n");
		goto error;
	}

	// If there aren't any clauses then initialize it with a start of 10
	if ( pat->containing_clauses == NULL )
	{
		pat->max_size = 10;
		pat->containing_clauses = (uint32_t*)malloc(sizeof(uint32_t) * 10);

		if ( pat->containing_clauses == NULL )
		{
			dprintf("[ERROR] Failed to initialize atom\n");
			goto error;
		}
		
		memset( pat->containing_clauses, 0x00, sizeof(uint32_t)*10);
		pat->count_neg = 0;
		pat->count_pos = 0;
		pat->count_total = 0;
		pat->max_size = 10;
		pat->val = 0;
		goto end;
	}

	new_max = pat->max_size * 2;
	new_clauses = (uint32_t*)malloc(sizeof(uint32_t) * new_max);

	if ( new_clauses == NULL )
	{
		dprintf("[ERROR] Failed to allocate new clauses array\n");
		goto error;
	}

	memset( new_clauses, 0x00, sizeof(uint32_t) * new_max);
	dprintf("[INFO] Atom will now hold %d clauses with max of %d\n",  pat->count_total, new_max);

	// Copy the old data
	memcpy( new_clauses, pat->containing_clauses, pat->count_total * sizeof(uint32_t) );

	free(pat->containing_clauses);
	pat->containing_clauses = new_clauses;
	pat->max_size = new_max;

	retval = 0;
	goto end;

error:
	retval = -1;

end:
	return retval;
}

int32_t expand_clause_atoms( pclause_t pct )
{
	int32_t retval = 0;
	uint32_t new_max = 0;
	int32_t *new_atoms = NULL;
	int8_t *new_vals = NULL;
	
	dprintf("[INFO] About to expand clause atoms\n");

	if (pct == NULL )
	{
		dprintf("[ERROR] NULL pointer in argument\n");
		goto error;
	}

	if ( pct->atoms == NULL || pct->atom_values == NULL )
	{
		dprintf("[ERROR] NULL pointer in structure\n");
		goto error;
	}

	if (pct->max_size == 0 )
	{
		pct->max_size = 3;
	}

	new_max = pct->max_size * 2;
	new_atoms = (int32_t*)malloc( sizeof(int32_t)*new_max );

	if ( new_atoms == NULL )
	{
		dprintf("[ERROR] Failed to allocate new atoms array\n");
		goto error;
	}

	new_vals = (int8_t*)malloc( sizeof(int8_t)*new_max);

	if ( new_vals == NULL )
	{
		dprintf("[ERROR] Failed to allocate atom values array.\n");
		goto error;
	}

	memset( new_atoms, 0x00, sizeof(int32_t)*new_max);
	memset( new_vals, 0x00, sizeof(int8_t)*new_max);

	memcpy( new_atoms, pct->atoms, (pct->num_atoms)*sizeof(int32_t));
	memcpy( new_vals, pct->atom_values, (pct->num_atoms)*sizeof(int8_t));


	free(pct->atoms);
	pct->atoms = new_atoms;
	pct->max_size = new_max;

	free(pct->atom_values);
	pct->atom_values = new_vals;

	goto end;

error:
	if ( new_atoms != NULL )
	{
		free(new_atoms);
	}

	if ( new_vals != NULL )
	{
		free(new_vals);
	}

	retval = -1;
end:
	return retval;
}

pdatabuff_t read_file( uint8_t *data )
{
    pdatabuff_t db = NULL;

    if ( data == NULL )
        return NULL;

    memset( db, 0x00, sizeof(databuff_t) );
    
    db->buffer = data;
    db->size = strlen( data );

    return db;
}

uint32_t seek_next_line( uint8_t *data, uint32_t max_len)
{
    uint32_t offset;

    if ( data == NULL )
    {
        dprintf("Invalid argument to seek next line\n");
        exit(-1);
    }

    for ( offset = 0; offset < max_len; offset++ )
    {
        // Check for CR LF
        if ( data[offset] == '\r' || data[offset] == '\n' )
        {
            if ( data[offset + 1] == '\r' || data[offset + 1] == '\n' )
            {
                offset += 2;
            } else
            {
                offset += 1;
            }

            break;
        }
    }

    return offset;
}

uint32_t seek_next_num( uint8_t *data, uint32_t max_len )
{
    uint32_t offset = 0;

    if ( data == NULL )
    {
        dprintf("Invalid argument to seek next num\n");
        exit(-1);
    }

    // Find the next delimiter
    while ( data[offset] != ' ' && offset < max_len ) { offset++; }

    // Skip over all spaces
    while ( (data[offset] != '-') && ( (data[offset] < '0') || (data[offset] > '9') ) && offset < max_len ) { offset++; }

    return offset;
}

