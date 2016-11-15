#ifndef __MYSCRIPT__
#define __MYSCRIPT__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef enum vartype_e
{
    _array = 0,
    _int8,
    _int16,
    _int32,
    _char,
} vartype;

typedef enum op_e
{
    _invalid = 0,
	_var,
    _yo,
	_pobastard, //int8
	_datbettah,	// int16
	_shebewalkinfunny, //int32
    _gimme,
    _makeit,
    _nomo,
	_moagin,	// add
	_eatdatshit,	// sub
	_splitdatshit, // div
	_screwdatshit,	// mul
    _putdatder,
    _slipitin,
    _slapitup,
    _oudahere,
	_dupme,
    _exite,
    _bandit,
    _borit,
    _bonut,
	_pullitout,
	_putitin,
    _deyseemeshiftind,
    _deyseemeshifting,
    _deyseemerollind,
    _deyseemerolling
} op;

// State for parsing
typedef enum state_e
{
	_start = 0,
	_varname,
	_needequ,
	_setvar,
	_op,
	_opargs,
	_argone,
	_argtwo,
	_argthree,
	_end
} state;

typedef struct nint8_t
{
	uint8_t val;
} nint8, *pnint8;

typedef struct nint16_t
{
	uint16_t val;
} nint16, *pnint16;

typedef struct nint32_t
{
	uint32_t val;
} nint32, *pnint32;

// This is an array type structure
typedef struct array_t
{
    uint32_t len;
    uint8_t data[0];
} array, *parray;

// This structure is used to store individual variables and their data
typedef struct var_t
{
    uint8_t *varname;
    uint8_t *vardata;
	vartype type;
} var, *pvar;

// This will store an array of all variables.
typedef struct varlist_t
{
	uint8_t *outbuff;
	uint32_t outlen;
    uint32_t numvars;
    uint32_t maxvars;
    pvar vars[0];
} varlist, *pvarlist;

uint32_t handle_yo( pvarlist pva, uint8_t *argone);
uint32_t handle_gimme( pvarlist pva, uint8_t*newvar, uint8_t*argone);
uint32_t handle_makeit( pvarlist pva, uint8_t*newvar, uint8_t*argone);
uint32_t handle_pobastard( pvarlist pva, uint8_t*newvar, uint8_t*argone);
uint32_t handle_datbettah( pvarlist pva, uint8_t*newvar, uint8_t*argone);
uint32_t handle_shebewalkinfunny( pvarlist pva, uint8_t*newvar, uint8_t*argone);
uint32_t handle_dupme( pvarlist pva, uint8_t*newvar, uint8_t*argone);
uint32_t handle_pullitout( pvarlist pva, uint8_t*newvar, uint8_t*argone, uint8_t*argtwo);
uint32_t handle_alg( pvarlist pva, uint8_t*dest, uint8_t*opone, uint8_t*optwo, uint32_t bin);
uint32_t handle_putdatder( pvarlist pva, uint8_t*argone, uint8_t*argtwo);
uint32_t handle_slipitin( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree);
uint32_t handle_slapitup( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree);
uint32_t handle_oudahere( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree);
uint32_t handle_putitin( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree);
uint32_t handle_binop( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree, uint32_t bin);
uint32_t handle_deyseeme( pvarlist pva, uint8_t*argone, uint8_t*argtwo, uint8_t*argthree, uint32_t bin);
uint32_t handle_bonut( pvarlist pva, uint8_t*argone, uint8_t*argtwo );
uint32_t handle_nomo( pvarlist pva, uint8_t * var );
uint32_t varExists( pvarlist pva, uint8_t *varname );
pvar getVar( pvarlist pva, uint8_t *varname );

#ifdef DEBUG
uint8_t *enum_to_string_op( op o );
#endif

uint32_t runscript( uint8_t *sc, uint32_t length, uint8_t *outbuff );

#endif
