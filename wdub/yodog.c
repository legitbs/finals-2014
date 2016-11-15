#include "wdub.h"

/*************
Types:    Ops:
int8     add, sub, mul, div, xor, or, not, shr, shl, ror, rol
int16   same as 8, not tochar but toarray
int32   same as 16
array   expand, delete, concat<array,char>, insert, overwrite, cut
char    same as int8, toint8, get_byte_from_array

Yo - print to stdout
newint = pobastard X - create int8 with value X
newint = datbettah X - int16
newint = shebewalkinfunny X - int32
putitin array int offset - overwrite an integer into the array at offset
x = pullitout array offset - copy an integer out of an array at offset
moagin - add
eatdatshit - sub
splitdatshit - div
screwdatshit - mul
newarr = gimme X - create an array of x size
newarr = makeit X - expand array to x size
nomo [varname] - delete array
putdatder [destarray] [srcarray] - concat
slipitin [destarray] [src] [start] - insert
slapitup [destarray] [src] [start] - overwrite
oudahere [array] [start] [size] - cut from array
exite [array] [offset] [value] - xor offset in array
bandit [array] [offset] [value ] - and offset in array
borit [array] [offset] [value]  - or offset in array
bonut [array] [offset] [value] - not offset in array
deyseemeshiftinr - shr offset in array
deyseemeshiftinl - shl offset in array
deyseemerollinr - ror
deyseemerollinl - rol

<1 byte type> <size [if array type]> <data>

*******/

#define MAX 0x200

uint32_t handle_gimme( pvarlist pva, uint8_t*newvar, uint8_t*argone)
{
    uint32_t retval = 0xffffffff;
    uint32_t size = 0;
    uint32_t counter = 0;
    uint32_t varname_len = 0;
    pvar pv = NULL;
    parray pa = NULL;
    uint32_t index = 0;

    if ( !pva || !newvar || !argone ) {
        goto end;
    }

    index = varExists( pva, newvar );

    // If the variable exists already I will not create another
    if ( index != retval ) {
        DEBUG_PRINT("[-] Variable %s already exists\n", newvar);
        goto end;
    }

    // Cheap isnum
    while ( argone[counter] ) {
        if ( argone[counter] < '0' || argone[counter] >'9'){
            DEBUG_PRINT("[-] gimme argone must be an integer\n");
            goto end;
        }
        counter++;
    }

    size = catoi( argone );

    if ( size > MAX ) {
        DEBUG_PRINT("[-] gimme size excedes bounds\n");
        goto end;
    }

    // Allocate an array structure with the correct buffer size hopefully
    pa = malloc( sizeof( array ) + size );

    if ( pa == NULL ) {
        goto end;
    }

    memset( pa, 0x00, sizeof(array) + size );

    // Allocate the variable management structure.
    pv = malloc( sizeof(var) );

    if ( pv == NULL ) {
        free(pa);
        goto end;
    }

    memset( pv, 0x00, sizeof(var));

    varname_len = strlen( (const char *)newvar );
    pv->varname = malloc( varname_len + 1 );
    pv->type = _array;

    if (pv->varname == NULL ) {
        free(pv);
        free(pa);
        goto end;
    }

    // Allocate and copy variable name
    memset( pv->varname, 0x00, varname_len + 1 );
    memcpy( pv->varname, newvar, varname_len );

    DEBUG_PRINT("[+] gimme: \n");

    pv->vardata = (uint8_t*)pa;

    pa->len = size;
    memset(pa->data, 0x00, pa->len);

    pva->vars[ pva->numvars ] = pv;
    pva->numvars++;

    retval = 0;
end:
    return retval;
}

pvar getVar( pvarlist pva, uint8_t *varname )
{
    pvar retval = 0;
    register uint32_t counter = 0;
    register uint32_t lenone = 0;
    uint32_t lentwo = 0;

    if ( !pva || !varname ) {
        goto end;
    }

    while ( counter < pva->numvars ) {
        if ( pva->vars[counter] == NULL ) {
            counter++;
            continue;
        }

        if ( varname[0] == pva->vars[counter]->varname[0] ) {
            lenone = strlen( (const char *) varname);
            lentwo = strlen((const char *)pva->vars[counter]->varname);
            if ( lenone == lentwo ) {
                if ( memcmp( varname, pva->vars[counter]->varname, lenone) == 0 ) {
                    retval = pva->vars[counter];
                    goto end;
                }
            }
        }

        counter++;
    }

end:
    return retval;
}

uint32_t varExists( pvarlist pva, uint8_t *varname )
{
    register uint32_t retval = 0xffffffff;
    register uint32_t counter = 0;
    register uint32_t lenone = 0;
    uint32_t lentwo = 0;

    if ( !pva || !varname ) {
        goto end;
    }

    while ( counter < pva->numvars ) {
        if ( pva->vars[counter] == NULL ) {
            counter++;
            continue;
        }

        if ( varname[0] == pva->vars[counter]->varname[0] ) {
            lenone = strlen( (const char *) varname);
            lentwo = strlen((const char *)pva->vars[counter]->varname);
            if ( lenone == lentwo ) {
                if ( !memcmp( varname, pva->vars[counter]->varname, lenone) ) {
                    retval = counter;
                    goto end;
                }
            }
        }

        counter++;
    }

end:
    return retval;
}

#ifdef DEBUG
uint8_t *enum_to_string_op( op o )
{
    switch( o ) {
        case _moagin:
            return (uint8_t*)"moagin";
            break;
        case _eatdatshit:
            return (uint8_t*)"eatdatshit";
            break;
        case _screwdatshit:
            return (uint8_t*)"screwdatshit";
            break;
        case _splitdatshit:
            return (uint8_t*)"splitdatshit";
            break;
        case _var:
            return (uint8_t*)"var";
            break;
        case _yo:
            return (uint8_t*)"Yo";
            break;
        case _gimme:
            return (uint8_t*)"gimme";
            break;
        case _makeit:
            return (uint8_t*)"makeit";
            break;
        case _nomo:
            return (uint8_t*)"nomo";
            break;
        case _putdatder:
            return (uint8_t*)"putdatder";
            break;
        case _slipitin:
            return (uint8_t*)"slipitin";
            break;
        case _slapitup:
            return (uint8_t*)"slapitup";
            break;
        case _oudahere:
            return (uint8_t*)"oudahere";
            break;
        case _exite:
            return (uint8_t*)"exite";
            break;
        case _dupme:
            return (uint8_t*)"dupme";
            break;
        case _bandit:
            return (uint8_t*)"bandit";
            break;
        case _borit:
            return (uint8_t*)"borit";
            break;
        case _bonut:
            return (uint8_t*)"bonut";
            break;
        case _pullitout:
            return (uint8_t*)"pullitout";
            break;
        case _putitin:
            return (uint8_t*)"putitin";
            break;
        case _deyseemeshiftind:
            return (uint8_t*)"deyseemeshiftind";
            break;
        case _deyseemeshifting:
            return (uint8_t*)"deyseemeshifting";
            break;
        case _deyseemerollind:
            return (uint8_t*)"deyseemerollind";
            break;
        case _deyseemerolling:
            return (uint8_t*)"deyseemerolling";
            break;
        case _pobastard:
            return (uint8_t*)"pobastard";
            break;
        case _datbettah:
            return (uint8_t*)"datbettah";
            break;
        case _shebewalkinfunny:
            return (uint8_t*)"shebewalkinfunny";
            break;
        default:
            return NULL;
            break;
    };

    return NULL;
}
#endif

op handle_Start( uint8_t *st ) 
{
    op retval = _var;
    register uint32_t length = 0;

    if ( st == NULL ) {
        goto end;
    }

    length = strlen((const char *)st);

    switch(length) {
        case 2:
            if ( memcmp( st, "Yo", length) == 0 ) {
                retval = _yo;
            }
            break;
        case 4:
            if ( memcmp( st, "nomo", length) == 0 ) {
                retval = _nomo;
            }
            break;
        case 5:
            if ( memcmp( st, "gimme", length) == 0 ) {
                retval = _gimme;
            } else if ( memcmp( st, "exite", length) == 0 ) {
                retval = _exite;
            } else if ( memcmp( st, "borit", length) == 0 ) {
                retval = _borit;
            } else if ( memcmp( st, "bonut", length) == 0 ) {
                retval = _bonut;
            } else if ( memcmp( st, "dupme", length) == 0 ) {
                retval = _dupme;
            }
            break;
        case 6:
            if ( memcmp( st, "makeit", length) == 0 ) {
                retval = _makeit;
            } else if ( memcmp( st, "bandit", length) == 0 ) {
                retval = _bandit;
            } else if ( memcmp( st, "moagin", length) == 0 ) {
                retval = _moagin;
            }
            break;
        case 7:
            if ( memcmp( st, "putitin", length) == 0 ) {
                retval = _putitin;
            }
        case 8:
            if ( memcmp( st, "slipitin", length) == 0 ) {
                retval = _slipitin;
            } else if ( memcmp( st, "slapitup", length) == 0 ) {
                retval = _slapitup;
            } else if ( memcmp( st, "oudahere", length) == 0 ) {
                retval = _oudahere;
            }
            break;
        case 9:
            if ( memcmp( st, "putdatder", length) == 0 ) {
                retval = _putdatder;
            } else if ( memcmp( st, "pullitout", length) == 0 ) {
                retval = _pullitout;
            } else if ( memcmp( st, "pobastard", length) == 0 ) {
                retval = _pobastard;
            } else if ( memcmp( st, "datbettah", length ) == 0 ) {
                retval = _datbettah;
            }
            break;
        case 10:
            if ( memcmp( st, "eatdatshit", length) == 0 ) {
                retval = _eatdatshit;
            }
            break;
        case 12:
            if ( memcmp( st, "splitdatshit", length) == 0 ) {
                retval = _splitdatshit;
            } else if ( memcmp( st, "screwdatshit", length) == 0 ) {
                retval = _screwdatshit;
            }
            break;
        case 15:
            if ( memcmp( st, "deyseemerollind", length)==0 ) {
                retval = _deyseemerollind;
            } else if ( memcmp( st, "deyseemerolling", length) == 0 ) {
                retval = _deyseemerolling;
            }
            break;
        case 16:
            if ( memcmp( st, "deyseemeshiftind", length) == 0 ) {
                retval = _deyseemeshiftind;
            } else if ( memcmp( st, "deyseemeshifting", length) == 0 ) {
                retval = _deyseemeshifting;
            } else if ( memcmp( st, "shebewalkinfunny", length) == 0 ) {
                retval = _shebewalkinfunny;
            }
            break;
        default:
            break;
    };
    
end:
    return retval;
}

/// I need something to parse a line
uint32_t parse_line( pvarlist pva, uint8_t*line)
{
    register uint32_t index = 0;
    register uint32_t retval = 0;
    register uint32_t start = 0;
    uint8_t *newvar = 0;
    uint8_t *argone = NULL;
    uint8_t *argtwo = NULL;
    uint8_t *argthree = NULL;
    state st = _start;
    op newop = _invalid;

    if ( line == NULL ) {
        goto end;
    }

    if ( pva == NULL ) {
        goto end;
    }

    // pass whitespace
    while ( line[index] == ' ' || line[index] == '\r' || line[index] == '\n') {
        index++;
    }

    start = index;

    // Get to the end of the first thing
    while ( line[index] != '\x00' ) {
        if ( line[index] == ' ' ) {
            line[index] = '\x00';
            if ( st == _start ) {
                DEBUG_PRINT("[+] Hit a space: state = _start\n");

                newop = handle_Start( line + start );

                if ( newop == _gimme || newop == _makeit || newop == _pobastard
                        || newop == _datbettah || newop == _shebewalkinfunny
                        || newop == _dupme || newop == _pullitout 
                        || newop == _moagin || newop == _eatdatshit
                        || newop == _screwdatshit || newop == _splitdatshit) {
                    DEBUG_PRINT("[-] gimme/makeit/int*/dupme/pullitout need '='\n");
                    goto end;
                } else if ( newop == _var ) {
                    newvar = line+start;
                    st = _needequ;
                } else {
                    st = _argone;
                }

                index++;

                while (line[index] == ' ') { index++; }

                start = index;
                continue;
            } else if (st == _op ) {
                DEBUG_PRINT("Hit a space: state = _op\n");
                newop = handle_Start( line + start );

                index++;

                while( line[index] == ' '){index++;}

                start = index;
                st = _argone;
                continue;
            } else if ( st == _setvar ) {
                DEBUG_PRINT("[+] Hit a space: state = _setvar\n");
                newop = handle_Start( line + start );

                if ( newop != _gimme && newop != _makeit && newop != _pobastard
                        && newop != _datbettah && newop != _shebewalkinfunny
                        && newop != _dupme && newop != _pullitout 
                        && newop != _moagin && newop != _eatdatshit
                        && newop != _splitdatshit && newop != _screwdatshit ) {
                    DEBUG_PRINT("Set var requires gimme makeit int8/16/32 dupme\n");
                    goto end;
                }

                index++;

                while(line[index]==' ') {index++;}

                start = index;
                st = _argone;    

                continue;
            } else if ( st == _argone ) {
                line[index] = '\x00';
                argone = line+start;
                st = _argtwo;
                index++;
                while( line[index] == ' ' ) {index++;}
                start = index;
            } else if ( st == _argtwo ) {
                if ( newop == _yo ||newop == _gimme
                    || newop == _makeit
                    || newop == _nomo
                    || newop == _pobastard
                    || newop == _datbettah
                    || newop == _shebewalkinfunny
                    || newop == _dupme ) {
                    DEBUG_PRINT("[-] %s only takes one arg\n", enum_to_string_op(newop));
                    goto end;
                }
                line[index] = '\x00';
                argtwo = line+start;
                st = _argthree;
                index++;
                while( line[index] == ' ' ) {index++;}
                start = index;
            } else if ( st == _argthree ) {
                line[index++] = '\x00';
                argthree = line+start;
                st = _end;
                while( line[index] == ' ' ) {index++;}
                start = index;
            } else if ( st == _end ) {
                DEBUG_PRINT("[-] Spaces should not follow end\n");
                goto end;
            } else {
                DEBUG_PRINT("Hit a space: state = other\n");
                index++;
                start = index;
                continue;
            }
        } else if ( line[index] == '=' ) {
            line[index] = '\x00';
            if ( st == _start ) {
                DEBUG_PRINT("Hit an equal: state = _start\n");
                newop = handle_Start( line + start );
                if ( newop != _var ) {
                    DEBUG_PRINT("[-] an '=' needs var\n");
                    goto end;
                }
                newvar = line+start;
                st = _setvar;
            } else if ( st == _needequ ) {
                DEBUG_PRINT("Hit an equal: state = _needequ\n");
                while ( line[index] == ' ' ) {index++;}
                start = index;
                st = _setvar;
            } else {
                DEBUG_PRINT("[+] Hit an equal: state = %d\n", st);
                DEBUG_PRINT("[-] why do i have an '='\n");
                goto end;
            }
            index++;
            while(line[index] == ' ') {index++;}
            start = index;
            continue;
        } else if ( line[index] == '\r' || line[index] == '\n' || line[index] == ';' ) {
            line[index] = '\x00';
            index++;
            if ( st == _argone ) {
                argone = line+start;
            } else if ( st == _argtwo ) {
                if ( newop == _yo || newop == _gimme
                    || newop == _nomo || newop == _makeit || newop == _pobastard
                    || newop == _datbettah || newop == _shebewalkinfunny 
                    || newop == _dupme
                    ) {
                    DEBUG_PRINT("[-] Op only has 1 arg\n");
                    goto end;
                }
                argtwo = line+start;
            } else if ( st == _argthree ) {
                if ( newop == _putdatder || newop == _pullitout ) {
                    DEBUG_PRINT("[-] op only has two args\n");
                    goto end;
                }
                argthree = line+start;
            } else {
                DEBUG_PRINT("[+] line finished\n");
            }

            while ( line[index] == '\n' || line[index] == '\r' || line[index] == ' ' ) { index++; }
            goto eval;
        } else {
            if ( st == _end ) {
                DEBUG_PRINT("[-] There shouldn't be anything else\n");
                goto end;
            } else if ( st == _needequ ) {
                DEBUG_PRINT("[-] This should be '='\n");
                goto end;
            }
            index++;
            continue;
        }
    }

eval:
#ifdef DEBUG
    if ( newvar ) {
        DEBUG_PRINT("Var: %s ", newvar);
    }

    DEBUG_PRINT("Op: %s ", enum_to_string_op( newop ));
    if ( argone ) { DEBUG_PRINT("Arg1: %s ", argone);}
    if ( argtwo ) { DEBUG_PRINT("Arg2: %s ", argtwo);}
    if ( argthree ) { DEBUG_PRINT("Arg3: %s", argthree);}
    DEBUG_PRINT("\n");
#endif
    if ( newop == _gimme ) {
        retval = handle_gimme( pva, newvar, argone );
    } else if ( newop == _makeit ) {
        retval = handle_makeit( pva, newvar, argone );
    } else if ( newop == _putdatder ) {
        retval = handle_putdatder( pva, argone, argtwo );
    } else if ( newop == _slipitin ) {
        retval = handle_slipitin( pva, argone, argtwo, argthree );
    } else if ( newop == _slapitup ) {
        retval = handle_slapitup( pva, argone, argtwo, argthree );
    } else if ( newop == _oudahere ) {
        retval = handle_oudahere( pva, argone, argtwo, argthree );
    } else if ( newop == _exite ) {
        retval = handle_binop( pva, argone, argtwo, argthree, 0 );
    } else if ( newop == _bandit ) {
        retval = handle_binop( pva, argone, argtwo, argthree, 1 );
    } else if ( newop == _borit ) {
        retval = handle_binop( pva, argone, argtwo, argthree, 2 );
    } else if ( newop == _bonut ) {
        retval = handle_bonut( pva, argone, argtwo );
    } else if ( newop == _nomo ) {
        retval = handle_nomo( pva, argone );
    } else if ( newop == _dupme ) {
        retval = handle_dupme( pva, newvar, argone );
    } else if ( newop == _deyseemeshiftind ) {
        retval = handle_deyseeme( pva, argone, argtwo, argthree, 0 );
    } else if ( newop == _deyseemeshiftind ) {
        retval = handle_deyseeme( pva, argone, argtwo, argthree, 1 );
    } else if ( newop == _deyseemerollind ) {
        retval = handle_deyseeme( pva, argone, argtwo, argthree, 2 );
    } else if ( newop == _deyseemerolling ) {
        retval = handle_deyseeme( pva, argone, argtwo, argthree, 3 );
    } else if ( newop == _pobastard ) {
        retval = handle_pobastard( pva, newvar, argone );
    } else if ( newop == _datbettah ) {
        retval = handle_datbettah( pva, newvar, argone );
    } else if ( newop == _shebewalkinfunny ) {
        retval = handle_shebewalkinfunny( pva, newvar, argone );
    } else if ( newop == _pullitout ) {
        retval = handle_pullitout( pva, newvar, argone, argtwo );
    } else if ( newop == _putitin ) {
        retval = handle_putitin( pva, argone, argtwo, argthree );
    } else if ( newop == _yo ) {
        retval = handle_yo( pva, argone );
    } else if ( newop == _moagin ) {
        retval = handle_alg( pva, newvar, argone, argtwo, 0 );
    } else if ( newop == _eatdatshit ) {
        retval = handle_alg( pva, newvar, argone, argtwo, 1 );
    } else if ( newop == _splitdatshit ) {
        retval = handle_alg( pva, newvar, argone, argtwo, 2 );
    } else if ( newop == _screwdatshit ) {
        retval = handle_alg( pva, newvar, argone, argtwo, 3 );
    } else {
        retval = 0xffffffff;
        DEBUG_PRINT("Op not implemented\n");
    }

    if ( retval != 0xffffffff ) {
        retval = index;
    } 
end:
    return retval;

}

uint32_t handle_alg( pvarlist pva, uint8_t *dest, uint8_t * opone, uint8_t *optwo, uint32_t bin ) {
    register uint32_t retval = 0xffffffff;
    pvar pv = NULL;
    pvar varone = NULL;
    pvar vartwo = NULL;
    register uint32_t counter = 0;
    uint32_t valone = 0;
    uint32_t valtwo = 0;
    uint32_t result = 0;

    if ( !pva || !dest || !opone || !optwo ) {
        goto end;
    }

    pv = getVar( pva, dest );
    varone = getVar( pva, opone );
    vartwo = getVar( pva, optwo );

    if ( varone == NULL ) {
        counter = 0;
        while ( opone[counter] ) {
            if ( opone[counter] < '0' || opone[counter] > '9' ) {
                DEBUG_PRINT("[-] opone must be an existing var or decimal\n");
                goto end;
            }
            counter++;
        }
        valone = catoi(opone);
    } else {
        switch( varone->type ) {
            case _int8:
                valone = ((pnint8)varone->vardata)->val;
                break;
            case _int16:
                valone = ((pnint16)varone->vardata)->val;
                break;
            case _int32:
                valone = ((pnint32)varone->vardata)->val;
                break;
            default:
                DEBUG_PRINT("[-] varone invalid type\n");
                goto end;
                break;
        };
    }    

    if ( vartwo == NULL ) {
        counter = 0;
        while ( optwo[counter] ) {
            if ( optwo[counter] < '0' || optwo[counter] > '9' ) {
                DEBUG_PRINT("[-] opone must be an existing var or decimal\n");
                goto end;
            }
            counter++;
        }
        valtwo = catoi( optwo);
    } else {
        switch( vartwo->type ) {
            case _int8:
                valtwo = ((pnint8)vartwo->vardata)->val;
                break;
            case _int16:
                valtwo = ((pnint16)vartwo->vardata)->val;
                break;
            case _int32:
                valtwo = ((pnint32)vartwo->vardata)->val;
                break;
            default:
                DEBUG_PRINT("[-] vartwo invalid type\n");
                goto end;
                break;
        };
    }    

    switch ( bin ) {
        case 0:
            result = valone + valtwo;
            break;
        case 1:
            result = valone - valtwo;
            break;
        case 2:
            result = valone * valtwo;
            break;
        case 3:
            result = valone / valtwo;
            break;
        default:
            goto end;
            break;
    };

    if ( pv == NULL ) {
        pv = malloc( sizeof( var ) );

        if ( pv == NULL ) {
            goto end;
        }

        counter = strlen( (const char *) dest );
        pv->varname = malloc ( counter + 1 );

        if ( pv->varname == NULL ) {
            free(pv);
            goto end;
        }

        memcpy( pv->varname, dest, counter );
        pv->varname[counter] = 0;
        pv->type = _int32;
        pv->vardata = malloc( sizeof(nint32));

        if ( pv->vardata == NULL ) {
            free(pv->varname);
            free(pv);
            goto end;
        }

        ((pnint32)pv->vardata)->val = result;

        pva->vars[ pva->numvars++] = pv;
    } else {
        switch( pv->type ) {
            case _int8:
                ((pnint8)pv->vardata)->val = result;
                break;
            case _int16:
                ((pnint16)pv->vardata)->val = result;
                break;
            case _int32:
                ((pnint32)pv->vardata)->val = result;
                break;
            default:
                DEBUG_PRINT("[-] dest invalid type\n");
                goto end;
                break;
        };
    }

    retval = 1;
    
end:
    return retval;
}

uint32_t handle_yo( pvarlist pva, uint8_t *argone )
{
    uint32_t retval = 0xfffffff;
    pvar pv = NULL;
    uint32_t length = 0;
    uint8_t *out = NULL;

    if ( !pva || !argone ) {
        DEBUG_PRINT("[-] yo invalid args\n");
        goto end;
    }

    pv = getVar( pva, argone );

    if ( pv == NULL ) {
        DEBUG_PRINT("[-] yo requires a valid variable\n");
        goto end;
    }

    out = pv->vardata;
    switch( pv->type ) {
        case _int8:
            length = 1;
            break;
        case _int16:
            length = 2;
            break;
        case _int32:
            length = 4;
            break;
        case _array:
            length = ( ((uint32_t*)pv->vardata)[0] > MAX ) ? MAX : ((uint32_t*)pv->vardata)[0];
            out += 4;
            break;
        default:
            goto end;
            break;
    };

    memcpy( pva->outbuff, out, length );
    pva->outlen = length;
    retval = 1;

end:
    return retval;
}

uint32_t handle_putitin( pvarlist pva, uint8_t *argone, uint8_t * argtwo, uint8_t *argthree )
{
    uint32_t retval = 0xffffffff;
    pvar pv = NULL;
    pvar pi = NULL;
    pvar ptwo = NULL;
    parray pa = NULL;
    uint32_t counter = 0;

    if ( !pva || !argone || !argtwo || !argthree ) {
        DEBUG_PRINT("[-] putitin invalid args\n");
        goto end;
    }

    pv = getVar( pva, argone );
    pi = getVar( pva, argtwo );
    ptwo = getVar( pva, argthree );

    if ( !pv || !pi ) {
        DEBUG_PRINT("[-] putitin requires argone and argtwo to exist\n");
        goto end;
    }

    if ( ptwo != NULL ) {
        switch ( ptwo->type ) {
            case _int8:
                counter = ((pnint8)ptwo->vardata)->val;
                break;
            case _int16:
                counter = ((pnint16)ptwo->vardata)->val;
                break;
            case _int32:
                counter = ((pnint32)ptwo->vardata)->val;
                break;
            default:
                DEBUG_PRINT("[-] pullitout invalid argthree\n");
                goto end;
                break;
        };
    } else {
        while ( argthree[counter] ) {
            if ( argthree[counter] < '0' || argthree[counter] > '9' ) {
                goto end;
            }
            counter++;
        }
        counter = catoi( argthree);
    }

    if ( pv->type != _array ) {
        DEBUG_PRINT("[-] argone must be an array\n");
        goto end;
    }

    if ( pi->type != _int8 && pi->type != _int16 && pi->type != _int32 ) {
        DEBUG_PRINT("[-] argtwo must be an int8/16/32\n");
        goto end;
    }

    pa = (parray)pv->vardata;

    switch ( pi->type ) {
        case _int8:
            if ( counter > pa->len ) {
                DEBUG_PRINT("[-] cannot write beyond buffer\n");
                goto end;
            }
            memcpy( pa->data + counter, &(((pnint8)pi->vardata)->val), 1 );
            break;
        case _int16:
            if ( counter+2 > pa->len ) {
                DEBUG_PRINT("[-] cannot write beyond buffer\n");
                goto end;
            }
            memcpy( pa->data+counter, &(((pnint16)pi->vardata)->val), 2 );
            break;
        case _int32:
            if ( counter+4 > pa->len ) {
                DEBUG_PRINT("[-] cannot write beyond buffer\n");
                goto end;
            }
            memcpy( pa->data+counter, &(((pnint32)pi->vardata)->val), 4 );
            break;
        default:
            goto end;
            break;
    };

    retval = 1;
end:
    return retval;
}

uint32_t handle_pullitout( pvarlist pva, uint8_t *newvar, uint8_t *argone, uint8_t * argtwo )
{
    uint32_t retval = 0xffffffff;
    pvar pv = NULL;
    pvar pi = NULL;
    pvar ptwo = NULL;
    pnint8 p8 = NULL;
    pnint16 p16 = NULL;
    pnint32 p32 = NULL;
    parray pa = NULL;
    uint32_t counter = 0;

    if ( !pva || !newvar || !argone || !argtwo ) {
        DEBUG_PRINT("[-] pullitout invalid args\n");
        goto end;
    }

    pv = getVar( pva, newvar );
    pi = getVar( pva, argone );
    ptwo = getVar( pva, argtwo );

    if ( !pv || !pi ) {
        DEBUG_PRINT("[-] pullitout requires newvar and argone to exist\n");
        goto end;
    }

    if ( pi->type != _array ) {
        DEBUG_PRINT("[-] argone must be an array\n");
        goto end;
    }

    if ( pv->type != _int8 && pv->type != _int16 && pv->type != _int32 ) {
        DEBUG_PRINT("[-] newvar must be an int8/16/32\n");
        goto end;
    }

    if ( ptwo != NULL ) {
        switch ( ptwo->type ) {
            case _int8:
                counter = ((pnint8)ptwo->vardata)->val;
                break;
            case _int16:
                counter = ((pnint16)ptwo->vardata)->val;
                break;
            case _int32:
                counter = ((pnint32)ptwo->vardata)->val;
                break;
            default:
                DEBUG_PRINT("[-] pullitout invalid argtwo\n");
                goto end;
                break;
        };
    } else {
        while ( argtwo[counter] ) {
            if ( !isdigit( argtwo[counter] ) ) {
                goto end;
            }
            counter++;
        }
        counter = catoi(argtwo);
    }

    pa = (parray)pi->vardata;

    switch ( pv->type ) {
        case _int8:
            p8 = (pnint8)pv->vardata;
            if ( counter > pa->len ) {
                DEBUG_PRINT("[-] cannot read beyond buffer %x > %x\n", counter, pa->len);
                goto end;
            }
            memcpy( &(p8->val), pa->data + counter, 1 );
            break;
        case _int16:
            p16 = (pnint16)pv->vardata;
            if ( counter+2 > pa->len ) {
                DEBUG_PRINT("[-] cannot read beyond buffer %x > %x\n", counter+2, pa->len);
                goto end;
            }
            memcpy( &p16->val, pa->data+counter, 2 );
            break;
        case _int32:
            p32 = (pnint32)pv->vardata;
            if ( counter+4 > pa->len ) {
                DEBUG_PRINT("[-] cannot read beyond buffer %x > %x\n", counter+4, pa->len);
                goto end;
            }
            memcpy( &p32->val, pa->data+counter, 4 );
            break;
        default:
            goto end;
            break;
    };

    retval = 1;
end:
    return retval;
}


uint32_t handle_dupme( pvarlist pva, uint8_t * newvar, uint8_t*argone)
{
    uint32_t retval = 0xffffffff;
    pvar pv = NULL;
    uint32_t varlen = 0;
    pvar ov = NULL;

    if ( !pva || !newvar || !argone ) {
        goto end;
    }

    pv = getVar( pva, newvar );    

    if ( pv != NULL ) {
        DEBUG_PRINT("[-] %s already exists\n", newvar);
        goto end;
    }

    ov = getVar( pva, argone );

    if ( ov == NULL ) {
        DEBUG_PRINT("[-] argone must be a valid var\n");
        goto end;
    }

    // Create a new var
    pv = malloc ( sizeof( var ) );

    if ( pv == NULL ) {
        goto end;
    }

    varlen = strlen( (const char *)newvar);

    pv->varname = malloc( varlen + 1 );

    DEBUG_PRINT("[+] dupme\n");

    if ( pv->varname == NULL ) {
        free(pv);
        goto end;
    }

    memcpy( pv->varname, newvar, varlen );
    pv->varname[varlen] = 0;

    pv->type = ov->type;
    pv->vardata    = ov->vardata;

    pva->vars[ pva->numvars++] = pv;
    retval = 1;
end:
    return retval;

}

uint32_t handle_shebewalkinfunny( pvarlist pva, uint8_t*newvar, uint8_t*argone)
{
    uint32_t retval = 0xffffffff;
    uint32_t index = 0;
    pnint32 newint = NULL;
    pvar pv = NULL;
    uint32_t nlen = 0;

    if ( !pva || !newvar || !argone ) {
        goto end;
    }

    index = varExists( pva, newvar );

    if ( index != retval ) {
        DEBUG_PRINT("[-] %s variable already exists\n", newvar);
        goto end;
    }

    pv = malloc( sizeof(var) );

    if ( pv == NULL ) {
        goto end;
    }

    nlen = strlen( (const char *)newvar);
    newint = malloc( sizeof(nint32) );

    if (newint == NULL ) {
        free(pv);
        goto end;
    }


    index = 0;

    while ( argone[index] ) {
        if ( argone[index] < '0' || argone[index] > '9' ) {
            DEBUG_PRINT("[-] int32 requires an integer argument\n");
            free(newint);
            free(pv);
            goto end;
        }
        index++;
    }

    index = catoi( argone);

    newint->val = index;

    pv->vardata = (uint8_t*)newint;
    pv->varname = malloc(nlen + 1);
    pv->type = _int32;

    if ( pv->varname == NULL ) {
        free(newint);
        newint = NULL;
        free(pv);
        pv = NULL;
        goto end;
    }

    DEBUG_PRINT("[+] int32\n");

    memset( pv->varname, 0x00, nlen + 1 );
    memcpy( pv->varname, newvar, nlen );

    pva->vars[ pva->numvars ] = pv;
    pva->numvars++;

    retval = 1;
end:
    return retval;
}

uint32_t handle_datbettah( pvarlist pva, uint8_t*newvar, uint8_t*argone)
{
    uint32_t retval = 0xffffffff;
    uint32_t index = 0;
    pnint16 newint = NULL;
    pvar pv = NULL;
    uint32_t nlen = 0;

    if ( !pva || !newvar || !argone ) {
        goto end;
    }

    index = varExists( pva, newvar );

    if ( index != retval ) {
        DEBUG_PRINT("[-] %s already exists\n", newvar);
        goto end;
    }

    newint = malloc( sizeof(nint16) );

    if (newint == NULL ) {
        goto end;
    }

    index = 0;

    while ( argone[index] ) {
        if ( argone[index] < '0' || argone[index] > '9' ) {
            DEBUG_PRINT("[-] int16 requires an integer argument\n");
            free(newint);
            newint = NULL;
            goto end;
        }
        index++;
    }

    index = catoi( argone);
    index <<= 16;
    index >>= 16;

    newint->val = index;

    pv = malloc( sizeof(var) );

    if ( pv == NULL ) {
        free(newint);
        newint = NULL;
        goto end;
    }

    nlen = strlen( (const char *)newvar);
    pv->vardata = (uint8_t*)newint;
    pv->varname = malloc(nlen + 1);
    pv->type = _int16;

    if ( pv->varname == NULL ) {
        free(newint);
        newint = NULL;
        free(pv);
        pv = NULL;
        goto end;
    }

    memset( pv->varname, 0x00, nlen + 1 );
    memcpy( pv->varname, newvar, nlen );

    pva->vars[ pva->numvars ] = pv;
    pva->numvars++;

    retval = 1;
end:
    return retval;
}

uint32_t handle_pobastard( pvarlist pva, uint8_t*newvar, uint8_t*argone)
{
    uint32_t retval = 0xffffffff;
    uint32_t index = 0;
    pnint8 newint = NULL;
    pvar pv = NULL;
    uint32_t nlen = 0;

    if ( !pva || !newvar || !argone ) {
        goto end;
    }

    index = varExists( pva, newvar );

    if ( index != retval ) {
        DEBUG_PRINT("[-] %s already exists\n", newvar);
        goto end;
    }

    newint = malloc( sizeof(nint8) );

    if (newint == NULL ) {
        goto end;
    }

    index = 0;

    while ( argone[index] ) {
        if ( argone[index] < '0' || argone[index] > '9' ) {
            DEBUG_PRINT("[-] int8 requires an integer argument\n");
            free(newint);
            newint = NULL;
            goto end;
        }
        index++;
    }

    index = catoi( argone) & 0xff;

    newint->val = index;

    pv = malloc( sizeof(var) );

    if ( pv == NULL ) {
        free(newint);
        newint = NULL;
        goto end;
    }

    nlen = strlen( (const char *)newvar);
    pv->vardata = (uint8_t*)newint;
    pv->varname = malloc(nlen + 1);
    pv->type = _int8;

    if ( pv->varname == NULL ) {
        free(newint);
        newint = NULL;
        free(pv);
        pv = NULL;
        goto end;
    }

    memset( pv->varname, 0x00, nlen + 1 );
    memcpy( pv->varname, newvar, nlen );

    pva->vars[ pva->numvars ] = pv;
    pva->numvars++;

    retval = 1;
end:
    return retval;
}

uint32_t handle_deyseeme( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree, uint32_t bin)
{
    uint32_t retval = 0xffffffff;
    pvar dest = NULL;
    parray da = NULL;
    pnint8 p8 = NULL;
    pnint16 p16 = NULL;
    pnint32 p32 = NULL;
    uint32_t counter = 0;
    uint32_t val = 0;
    uint32_t bit = 0;

    if ( !pva || !argone || !argtwo ) {
        goto end;
    }

    dest = getVar( pva, argone );

    if ( dest == NULL ) {
        DEBUG_PRINT("[-] %s does not exists\n", argone);
        goto end;
    }

    while ( argtwo[counter] ) {
        if ( argtwo[counter] < '0' || argtwo[counter] > '9' ) {
            DEBUG_PRINT("[-] deseeme* requires arg2 to be an integer\n");
            goto end;
        }
        counter++;
    }

    counter = catoi( argtwo);

    if ( dest->type == _int8 ) {
        p8 = (pnint8)dest->vardata;
        switch ( bin ) {
            case 0:
                p8->val >>= counter;
                break;
            case 1:
                p8->val <<= counter;
                break;
            case 2:
                while ( argtwo ) {
                    bit = p8->val & 1;
                    p8->val >>=1;
                    p8->val |= bit << 7;
                    argtwo--;
                }
                break;
            case 3:
                while ( argtwo ) {
                    bit = p8->val & 0x80;
                    p8->val <<= 1;
                    p8->val |= bit>>7;
                    argtwo--;
                }
                break;
            default:
                goto end;
                break;
        }
        retval = 1;
        goto end;
    } else if ( dest->type == _int16 ) {
        p16 = (pnint16)dest->vardata;
        switch ( bin ) {
            case 0:
                p16->val >>= counter;
                break;
            case 1:
                p16->val <<= counter;
                break;
            case 2:
                while ( argtwo ) {
                    bit = p16->val & 1;
                    p16->val >>=1;
                    p16->val |= bit << 15;
                    argtwo--;
                }
                break;
            case 3:
                while ( argtwo ) {
                    bit = p16->val & 0x8000;
                    p16->val <<= 1;
                    p16->val |= bit>>15;
                    argtwo--;
                }
                break;
            default:
                goto end;
                break;
        }
        retval = 1;
        goto end;
    } else if ( dest->type == _int32 ) {
        p32 = (pnint32)dest->vardata;
        switch ( bin ) {
            case 0:
                p32->val >>= counter;
                break;
            case 1:
                p32->val <<= counter;
                break;
            case 2:
                while ( argtwo ) {
                    bit = p32->val & 1;
                    p32->val >>=1;
                    p32->val |= bit << 31;
                    argtwo--;
                }
                break;
            case 3:
                while ( argtwo ) {
                    bit = p32->val & 0x80000000;
                    p32->val <<= 1;
                    p32->val |= bit>>31;
                    argtwo--;
                }
                break;
            default:
                goto end;
                break;
        }
        retval = 1;
        goto end;
    }

    if ( dest->type != _array ) {
        DEBUG_PRINT("[-] deyseeme must be array at this point\n");
        goto end;
    }

    if ( !argthree ) {
        DEBUG_PRINT("[-] deyseeme array ops require argthree\n");
        goto end;
    }

    da = (parray)dest->vardata;

    counter = 0;
    while( argthree[counter] ) {
        if ( argthree[counter] < '0' || argthree[counter] > '9' ) {
            DEBUG_PRINT("[-] deseemee arg three must be an integer\n");
            goto end;
        }
        counter++;
    }

    counter = catoi( argtwo );
    val = catoi( argthree);

    if ( counter > da->len ) {
        DEBUG_PRINT("[-] deyseemee cannot modify a value beyond buffer\n");
        goto end;
    }

    switch ( bin ) {
        case 0:
            da->data[ counter ] >>= (val & 0xff);
            break;
        case 1:
            da->data[ counter ] <<= (val & 0xff);
            break;
        case 2:
            while ( val ) {
                bit = da->data[counter]&1;
                da->data[ counter ] >>= 1;
                da->data[counter] |= bit << 7;
                val--;
            }
            break;
        case 3:
            while ( val ) {
                bit = da->data[counter]&0x80;
                da->data[counter] <<=1;
                da->data[counter] |= bit>>7;
                val--;
            }
            break;
        default:
            goto end;
            break;
    };

    retval = 1;
end:
    return retval;
}

uint32_t handle_bonut( pvarlist pva, uint8_t*argone, uint8_t*argtwo )
{
    uint32_t retval = 0xffffffff;
    pvar dest = NULL;
    parray da = NULL;
    pnint8 p8 = NULL;
    pnint16 p16 = NULL;
    pnint32 p32 = NULL;
    uint32_t counter = 0;

    if ( !pva || !argone ) {
        goto end;
    }

    dest = getVar( pva, argone );

    if ( dest == NULL ) {
        DEBUG_PRINT("[-] %s does not exist\n", argone);
        goto end;
    }

    if ( dest->type == _int8 ) {
        p8 = (pnint8)dest->vardata;
        p8->val = ~p8->val;
        retval = 1;
        goto end;
    } else if ( dest->type == _int16 ) {
        p16 = (pnint16)dest->vardata;
        p16->val = ~p16->val;
        retval = 1;
        goto end;
    } else if ( dest->type == _int32 ) {
        p32 = (pnint32)dest->vardata;
        p32->val = ~p32->val;
        retval = 1;
        goto end;
    }

    if ( dest->type != _array ) {
        DEBUG_PRINT("[-] At this point %s must be an array\n", argone);
        goto end;
    }

    if ( !argtwo ) {
        DEBUG_PRINT("[-] bonut array requires argtwo\n");
        goto end;
    }

    da = (parray)dest->vardata;

    while ( argtwo[counter] ) {
        if ( argtwo[counter] < '0' || argtwo[counter] > '9' ) {
            DEBUG_PRINT("[-] bonut requires argtwo to be integer\n");
            goto end;
        }
        counter++;
    }

    if ( counter > da->len ) {
        DEBUG_PRINT("[-] bonut cannot go beyond buffer\n");
        goto end;
    }

    counter = catoi( argtwo );

    da->data[counter] = ~da->data[counter];

    retval = 1;
end:
    return retval;
}

uint32_t handle_binop( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree, uint32_t bin)
{
    uint32_t retval = 0xffffffff;
    pvar dest = NULL;
    parray da = NULL;
    pnint8 p8 = NULL;
    pnint16 p16 = NULL;
    pnint32 p32 = NULL;
    uint32_t counter = 0;
    uint32_t val = 0;

    if ( !pva || !argone || !argtwo ) {
        goto end;
    }

    dest = getVar( pva, argone );

    if ( dest == NULL ) {
        DEBUG_PRINT("[-] %s does not exist\n", argone);
        goto end;
    }


    while ( argtwo[counter] ) {
        if ( argtwo[counter] < '0' || argtwo[counter] > '9' ) {
            DEBUG_PRINT("[-] binop requires argtwo to be an integer\n");
            goto end;
        }
        counter++;
    }

    counter = catoi( argtwo );

    if ( dest->type == _int8 ) {
        p8 = (pnint8)dest->vardata;
        switch( bin ) {
            case 0:
                p8->val ^= (counter & 0xff);
                break;
            case 1:
                p8->val &= (counter & 0xff);
                break;
            case 2:
                p8->val |= (counter &0xff);
                break;
            default:
                goto end;
                break;
        };
        retval = 1;
        goto end;
    } else if ( dest->type == _int16 ) {
        p16 = (pnint16)dest->vardata;
        counter <<= 16;
        counter >>= 16;
        switch( bin ) {
            case 0:
                p16->val ^= counter;
                break;
            case 1:
                p16->val &= counter;
                break;
            case 2:
                p16->val |= counter;
                break;
            default:
                goto end;
                break;
        };
        retval = 1;
        goto end;

    } else if ( dest->type == _int32 ) {
        p32 = (pnint32)dest->vardata;
        switch( bin ) {
            case 0:
                p32->val ^= counter;
                break;
            case 1:
                p32->val &= counter;
                break;
            case 2:
                p32->val |= counter;
                break;
            default:
                goto end;
                break;
        };
        retval = 1;
        goto end;
    }

    if ( dest->type != _array ) {
        DEBUG_PRINT("[-] At this point %s must be an array\n", argone);
        goto end;
    }

    counter = 0;
    while( argthree[counter] ) {
        if ( argthree[counter] < '0' || argthree[counter] > '9' ) {
            DEBUG_PRINT("[-] argthree in binop must be an integer\n");
            goto end;
        }
        counter++;
    }

    counter = catoi( argtwo );
    val = catoi( argthree);

    da = (parray)dest->vardata;

    if ( counter > da->len ) {
        DEBUG_PRINT("[-] binop cannot mod a value beyond buffer\n");
        goto end;
    }

    switch ( bin ) {
        case 0:
            da->data[ counter ] ^= (val & 0xff);
            break;
        case 1:
            da->data[ counter ] &= (val & 0xff);
            break;
        case 2:
            da->data[ counter ] |= (val &0xff);
            break;
        default:
            goto end;
            break;
    };

    retval = 1;
end:
    return retval;
}

uint32_t handle_oudahere( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree)
{
    uint32_t retval = 0xffffffff;
    pvar dest = NULL;
    parray da = NULL;
    uint32_t counter = 0;
    uint32_t size = 0;
    parray pa_new = NULL;

    if ( !pva || !argone || !argtwo ) {
        goto end;
    }

    dest = getVar( pva, argone );

    if ( dest == NULL ) {
        DEBUG_PRINT("[-] oudahere arg one var must exist\n");
        goto end;
    }

    da = (parray)dest->vardata;

    while ( argtwo[counter] ) {
        if ( argtwo[counter] < '0' || argtwo[counter] > '9' ) {
            DEBUG_PRINT("[-] oudahere arg two must be integer\n");
            goto end;
        }
        counter++;
    }

    counter = 0;
    while( argthree[counter] ) {
        if ( argthree[counter] < '0' || argthree[counter] > '9' ) {
            DEBUG_PRINT("[-] oudahere arg three must be an integer\n");
            goto end;
        }
        counter++;
    }

    counter = catoi( argtwo );
    size = catoi( argthree);

    if ( counter + size > da->len ) {
        goto end;
    }

    pa_new = malloc( sizeof(array) + (da->len - size) );

    if ( pa_new == NULL ) {
        goto end;
    }

    pa_new->len = da->len - size;

    memcpy( pa_new->data, da->data, counter );
    memcpy( pa_new->data + counter, da->data + counter + size, da->len - (counter + size ) );
    
    free(dest->vardata);
    dest->vardata = (uint8_t*)pa_new;

    retval = 1;
end:
    return retval;
}

uint32_t handle_putdatder( pvarlist pva, uint8_t *argone, uint8_t *argtwo )
{
    uint32_t retval = 0xffffffff;
    pvar dest = NULL;
    pvar src = NULL;
    uint32_t newsize = 0;
    parray da = NULL;
    parray sa = NULL;
    parray newdata = NULL;

    if ( !pva || !argone || !argtwo ) {
        goto end;
    }

    dest = getVar( pva, argone );
    src = getVar( pva, argtwo );

    if ( !dest || !src ) {
        DEBUG_PRINT("[-] putdatder requires two existing array varis\n");
        goto end;
    }

    if ( dest->type != _array || src->type != _array ) {
        DEBUG_PRINT("[-] Both vars must be arrays\n");
        goto end;
    }

    da = (parray)dest->vardata;
    sa = (parray)src->vardata;

    newsize = da->len + sa->len;

    if ( newsize > MAX ) {
        DEBUG_PRINT("[-] putdatder would exceed max len\n");
        goto end;
    }

    newdata = malloc( sizeof(array) + newsize );

    if ( newdata == NULL ) {
        goto end;
    }

    memcpy( newdata->data, da->data, da->len);
    memcpy( newdata->data + da->len, sa->data, sa->len);

    newdata->len = newsize;

    free(da);
    dest->vardata = (uint8_t*)newdata;

    retval = 0;

end:
    return retval;
}

uint32_t handle_nomo( pvarlist pva, uint8_t * var )
{
    uint32_t retval = 0xffffffff;
    uint32_t index = 0;
    pvar pv = NULL;
    
    if ( !pva || !var ) {
        goto end;
    }

    pv = getVar( pva, var );

    if ( pv == NULL ) {
        DEBUG_PRINT("[-] nomo %s does not exist\n", var);
        goto end;
    }

    index = varExists( pva, var );

    free( pva->vars[index]->vardata );
    free( pva->vars[index]->varname );
    free( pva->vars[index] );
    

    pva->vars[index] = NULL;


    retval = 0;
end:
    return retval;
}

uint32_t handle_makeit( pvarlist pva, uint8_t *var, uint8_t *argone)
{
    uint32_t retval = 0xffffffff;
    uint32_t size = 0;
    pvar toexpand = NULL;
    uint32_t counter = 0;
    parray pa_old = NULL;
    parray pa_new = NULL;

    if ( !pva || !var || !argone ) {
        goto end;
    }

    toexpand = getVar( pva, var );

    if ( toexpand == NULL ) {
        DEBUG_PRINT("[-] Cannot expand a non existant var\n");
        goto end;
    }

    if ( toexpand->type != _array ) {
        DEBUG_PRINT("[-] Cannot expand a non-array var\n");
        goto end;
    }

    while ( argone[counter] ) {
        if ( argone[counter] < '0' || argone[counter] >'9'){
            DEBUG_PRINT("[-] Invalid argument to makeit\n");
            goto end;
        }
        counter++;
    }

    size = catoi( argone );

    pa_old = (parray)toexpand->vardata;

    if ( size < pa_old->len ) {
        DEBUG_PRINT("[-] makeit only expands not contracts\n");
        goto end;
    }

    if ( size > MAX ) {
        DEBUG_PRINT("[-] Max size is 0x200\n");
        goto end;
    }


    pa_new = malloc( sizeof(array) + size );
    pa_new->len = size;
    memcpy( pa_new->data, pa_old->data, pa_old->len );

    free(toexpand->vardata);

    toexpand->vardata = (uint8_t*)pa_new;

    DEBUG_PRINT("[+] Expanded %s to %d bytes\n", toexpand->varname, pa_new->len);
    retval = 1;
end:
    return retval;
}

void print_vars( pvarlist pva )
{
    uint32_t counter = 0;
    parray pa = NULL;
    uint32_t i = 0;

    if ( pva == NULL ) {
        goto end;
    }

    while ( counter < pva->numvars ) {
        if ( pva->vars[counter] == NULL ) {
            counter++;
            continue;
        }
        DEBUG_PRINT("%d: %s - ", counter, pva->vars[counter]->varname);
        if ( pva->vars[counter]->type == _array) {
            DEBUG_PRINT("array\n");
            pa = (parray)pva->vars[counter]->vardata;
            DEBUG_PRINT("\tlen: %d\n", pa->len);
            DEBUG_PRINT("\tdata: ");
            for (i =0; i < pa->len; i++) {
                DEBUG_PRINT("%x ", pa->data[i]);
                if ( i > 0x20 ) {
                    DEBUG_PRINT("... to %d bytes", pa->len);
                    break;
                }
            }
            DEBUG_PRINT("\n\n");
        } else if ( pva->vars[counter]->type == _int8) {
            DEBUG_PRINT("int8\n");
            DEBUG_PRINT("\tval: %d\n", ((pnint8)pva->vars[counter]->vardata)->val);
            DEBUG_PRINT("\n");
        } else if ( pva->vars[counter]->type == _int16) {
            DEBUG_PRINT("int16\n");
            DEBUG_PRINT("\tval: %d\n", ((pnint16)pva->vars[counter]->vardata)->val);
            DEBUG_PRINT("\n");
        } else if ( pva->vars[counter]->type == _int32) {
            DEBUG_PRINT("int32\n");
            DEBUG_PRINT("\tval: %.8x\n", ((pnint32)pva->vars[counter]->vardata)->val);
            DEBUG_PRINT("\n");
        }
        counter++;
    }    

end:
    return;
}

uint32_t runscript( uint8_t *sc, uint32_t length, uint8_t *outbuff )
{
    uint32_t counter = 0;
    uint32_t retval = 0;
    uint32_t line_counter = 1;
    
    pvarlist pva = malloc( sizeof(varlist) + (sizeof(pvar)*100));

    if ( pva == NULL ) {
        DEBUG_PRINT("Failed to allocate variable array\n");
        return 0;
    }

    if ( outbuff == NULL ) {
        goto end;
    }

    memset( pva, 0x00, sizeof(varlist) + (sizeof(pvar)*100));
    pva->maxvars = 100;
    pva->outbuff = outbuff;
    pva->outlen = 0;

    while ( sc[counter] != 0x00 && counter < length) {
        retval = parse_line( pva, sc + counter );
        if ( retval == 0xffffffff ) {
            DEBUG_PRINT("[-] Error on line %d\n", line_counter);
            goto end;
        }

        counter += retval;        
        line_counter++;
    }

    retval = pva->outlen;
end:
    print_vars( pva );
    return retval;
}

uint32_t handle_slipitin( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree)
{
    uint32_t retval = 0xffffffff;
    pvar dest = NULL;
    pvar src = NULL;
    uint32_t newsize = 0;
    parray da = NULL;
    parray sa = NULL;
    parray pa_new = NULL;
    uint32_t counter = 0;

    if ( !pva || !argone || !argtwo || !argthree ) {
        goto end;
    }

    while ( argthree[counter] ) {
        if ( argthree[counter] < '0' || argthree[counter] > '9') {
            DEBUG_PRINT("[-] slipitin requires integer arg3\n");
            goto end;
        }
        counter++;
    }

    counter = catoi( argthree);

    dest = getVar( pva, argone );
    src = getVar( pva, argtwo );

    if ( !dest || !src ) {
        DEBUG_PRINT("[-] slipitin requires two valid vars\n");
        goto end;
    }

    if ( dest->type != _array || src->type != _array ) {
        DEBUG_PRINT("[-] slipitin requires two arrays\n");
        goto end;
    }

    da = (parray)dest->vardata;
    sa = (parray)src->vardata;

    // If I remove this check if may be an interesting bug NOTE THIS HERE
    // Ensure the insert offset is not beyond the length of the dest
    if ( counter > da->len ) {
        DEBUG_PRINT("[-] Insert offset beyond buffer end\n");
        goto end;
    }

    newsize = da->len + sa->len;

    if ( newsize > MAX ) {
        DEBUG_PRINT("[-] slipitin woudl exceed mx size\n");
        goto end;
    }

    pa_new = malloc( sizeof(array) + newsize );

    if ( pa_new == NULL ) {
        goto end;
    }

    pa_new->len = newsize;
    
    // Copy the initial data up to offset
    memcpy( pa_new->data, da->data, counter );

    // Copy the data to insert
    memcpy( pa_new->data + counter, sa->data, sa->len );

    // Finish it off
    memcpy( pa_new->data + counter + sa->len, da->data + counter, da->len - counter);
    
    free(dest->vardata);
    dest->vardata = (uint8_t*)pa_new;

    retval = 0;
end:
    return retval;
}

uint32_t handle_slapitup( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree)
{
    uint32_t retval = 0xffffffff;
    pvar dest = NULL;
    pvar src = NULL;
    pvar off = NULL;
    parray da = NULL;
    parray sa = NULL;
    uint32_t counter = 0;

    if ( !pva || !argone || !argtwo || !argthree ) {
        goto end;
    }

    dest = getVar( pva, argone );
    src = getVar( pva, argtwo );
    off = getVar( pva, argthree);

    if ( !dest || !src ) {
        DEBUG_PRINT("[-] slipitin requires two valid vars\n");
        goto end;
    }

    if ( off != NULL ) {
        switch ( off->type ) {
            case _int8:
                counter = ((pnint8)off->vardata)->val;
                break;
            case _int16:
                counter = ((pnint16)off->vardata)->val;
                break;
            case _int32:
                counter = ((pnint32)off->vardata)->val;
                break;
            default:
                DEBUG_PRINT("[-] slipitin invalid argthree\n");
                goto end;
                break;
        };
    } else {
        while ( argthree[counter] ) {
            if ( argthree[counter] < '0' || argthree[counter] > '9' ) {
                goto end;
            }
            counter++;
        }
        counter = catoi( argthree);
    }


    if ( dest->type != _array || src->type != _array ) {
        DEBUG_PRINT("[-] slipitin requires two arrays\n");
        goto end;
    }

    da = (parray)dest->vardata;
    sa = (parray)src->vardata;

    // If I remove this check if may be an interesting bug NOTE THIS HERE
    // Ensure the overwrite start does not go beyond the length of the dest
    if ( counter > da->len ) {
        DEBUG_PRINT("[-] Insert offset beyond buffer end\n");
        goto end;
    }

    // Ensure we do not go beyond the buffer
    if ( counter + sa->len > da->len ) {
        goto end;
    }

    memcpy( da->data + counter, sa->data, sa->len );

    retval = 1;
end:
    return retval;
}
