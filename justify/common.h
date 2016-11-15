#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NEG 1
#define POS 2
#define BOTH 1 | 2

#ifdef DEBUG
#define dprintf(...)    printf(__VA_ARGS__)
#else
#define dprintf(...)    
#endif

typedef struct cl {
    struct cl *next;
    int cv;
} cl, *pcl;

typedef struct dl {
    struct dl *next;
    int dv;
} dl, *pdl;


/// Structure for the decision stack
typedef struct decision_stack
{
    /// Pointer to the previous decision
    struct decision_stack * prev;

    /// Decided atom
    uint32_t atom;

    /// Decision level
    uint32_t level;

    /// Indicates if each choice (T) (F) has been attempted.
    uint32_t attempts;

} decision_stack_t, *pdecision_stack_t;

/// Implied variable stack
typedef struct implied_vars_stack
{
    /// Number of implied variables
    uint32_t implied_count;

    /// Index to process
    uint32_t index;

    /// Max number at a given time
    uint32_t max_len;

    /// Array of implied vars
    uint32_t *implied_vars;

    /// Implied var values
    int8_t *implied_vars_values;
 
} implied_vars_stack, *pimplied_vars_stack;

/// New structure for decisions
typedef struct new_decision_stack
{
    /// Current decision level
    uint32_t decision_level;

    /// Max array size
    uint32_t max_len;

    /// Array of decisions
    uint32_t *decision_list;

    /// Array of decision values
    int8_t *decision_list_values;

} nds, *pnds;

/// Structure to contain the propagated list.
typedef struct prop_list
{
    /// Atom that was propogated.
    uint32_t atom;

    /// Decision level at which it was propagated.
    uint32_t level;

    /// Set to 1 if this is an implied variable.
    uint8_t implied;

    /// If implied, which clause
    uint32_t implied_clause;

    /// Link to the previously propogated variable.
    struct prop_list *prev;
} prop_list_t, *pprop_list_t;

/// Structure to manage the implication graph
typedef struct implication_graph
{
    /// Implied literal
    int32_t imp_list;

    /// Clause causing the implication
    uint32_t clause;

    /// Array of literals causing this implication
    struct implication_graph **parent_implications;
} implication_graph_t, *pimplication_graph_t;

/// Structure for the implied variables that must be propagated.
typedef struct implied_vars
{
    /// Implied atom
    int32_t atom;

    /// Clause causing the implication
    uint32_t clause;

    /// link to the next implication.
    struct implied_vars *next;
} implied_vars_t, *pimplied_vars_t;

/// Structure to store data buffers
typedef struct databuff
{
    /// Pointer to data
    uint8_t *buffer;

    /// size of the buffer pointed to by buffer
    uint32_t size;

    /// Current offset if parsing through the data
    uint32_t offset;

} databuff_t, *pdatabuff_t;

/// Structure to store clause information
typedef struct clause
{
    /// Total number of atoms in a clause
    uint32_t num_atoms;

	/// Array size
	uint32_t max_size;

    /// array of the atoms in the clause
    int32_t *atoms;

    /// array of the values in the atom -1 - false; 0 - unset; 1 - true
    int8_t *atom_values;

    /// Number of undecided atoms
    int32_t unset;

    /// Clause value. Only important if it is true or if it is unset when unset = 1;
    int8_t value;

} clause_t, *pclause_t;

/// Structure to store atom information
typedef struct atom
{
    /// Atom setting 0 not set, <0 false, >0 true
    int8_t val;

    /// Number of clauses the atom is in in which it is positive
    uint32_t count_pos;
    
    /// Number of clauses the atom is in in which it is negative
    uint32_t count_neg;

    // Total number of clauses containing this literal
    uint32_t count_total;

    /// array of clauses containing this atom
    uint32_t *containing_clauses;

    /// size of containing_clauses array
    uint32_t max_size;

    /// Decision level where this atom was set. 0 means not decided yet.
    uint32_t decision_level;

    /// Antecedant where this atom was implied, 0 for a decision
    int32_t antecedant;

} atom_t, *patom_t;

/// Structure to organize atom/clause information
typedef struct meta
{
    /// Pointer to an array of atoms
    patom_t atoms;

    /// Number of atoms in the array
    uint32_t num_atoms;

    /// Pointer to an array of clauses
    pclause_t clauses;

    /// Number of clauses in the array
    uint32_t num_clauses;

    /// Maximum number of clauses that can be stored
    uint32_t max_clauses;
    
    /// socket descriptor
    uint32_t sockfd;

    /// Most common literal
    uint32_t high_lit;

    /// Most commong literal count
    uint32_t high_cnt;

	/// The number of conflicts that have occurred since the last backtrace
	uint32_t cslbt;

} meta_t, *pmeta_t;

/**
 * Parses the supplied DIMACS data into understandable clauses
 * @param data Pointer to buffer structure with DIMACS data.
 * @return Returns a structure containing clause and atom data.
 */
pmeta_t parse_dimacs( pdatabuff_t data, int connfd );

/**
 * Opens the specified file and reads in the data.
 * @param name Pointer to a string containing the name of the file to be opened.
 * @return Returns a pointer to a buffer structure containing the data and size
 */
pdatabuff_t read_file( uint8_t *name );

/**
 * Seeks to the next line in the DIMACS buffer
 * @param data Dimacs buffer data
 * @param max_len Maximum length of the data from the current pointer.
 * @return Returns the offset of the next line.
 */
uint32_t seek_next_line( uint8_t *data, uint32_t max_len );

/**
 * Seeks the start of the next numeric value
 * @param data Dimacs buffer data
 * @param max_len Maximum length of the data from the current pointer.
 * @return Returns the offset of the next numeric value
 */
uint32_t seek_next_num( uint8_t *data, uint32_t max_len );

/**
 * Propagates decision information
 * @param pmt Pointer to the atoms and clauses structures.
 * @param ivs Pointer to the implied variable stack
 * @param implied Set to 1 if this propagation is due to implication 0 otherwise.
 * @return Returns 1 if successful 0 otherwise
 */
int32_t binary_clause_propagation( pmeta_t pmt, pnds ds, pimplied_vars_stack ivs, uint32_t implied );

/**
 * Lists clauses associated with an atom.
 * @param pmt Pointer to the atoms and clauses structures
 * @param clause Clause to be listed
 * @return Returns 0 if the clause does not exist, 1 otherwise
 */
uint32_t display_clause( pmeta_t pmt, uint32_t clause );

/**
 * Lists atoms associated with a clause.
 * @param pmt Pointer to the atoms and clauses structures
 * @param atom Atom to be listed
 * @return Returns 0 if the atom does not exist, 1 otherwise
 */
uint32_t display_atom( pmeta_t pmt, uint32_t atom );

/**
 * Handles the propagation of implied variables
 * @param pmt Pointer to the atoms and clauses structures
 * @pram ds Pointer to the decision stack
 * @param ivs Pointer to the implied vars
 * @return Returns 0 if there is a conflict 1 otherwise.
 */
uint32_t propogate_implied( pmeta_t pmt, pnds ds, pimplied_vars_stack ivs );

/**
 * Cleans up after a conflict
 * @param pmt Pointer to the atoms and clauses structures
 * @param ds Pointer to the decision stack
 * @return returns 0 if unsuccessful, 1 otherwise
 */
uint32_t cleanup_conflict( pmeta_t pmt, pnds ds );

/**
 * Make a decision
 * @param pmt Pointer to the atoms and clauses structures
 * @param ds Pointer to the decision stack
 * @param conflict Set to 1 if this is a conflic resolution
 * @return returns the decision level. -1 on failure, 0 on solved.
 */
int32_t decide( pmeta_t pmt, pnds ds, int conflict );

/**
 * Reallocate the size of the clauses array in the meta structure
 * @param pmt Pointer to the atoms and clauses structures
 * @return returns 0 on success, -1 on failure
 */
int32_t realloc_clause_array( pmeta_t pmt );

/**
 * Increase the number of atoms that a clause can hold
 * @param pct Pointer to the clause to resize
 * @return returns 0 on success, -1 on failure
 */
int32_t expand_clause_atoms( pclause_t pct );

/**
 * Expand the number of clauses that an atom can hold
 * @param pat Pointer to an atom to resize
 * @return returns 0 on success, -1 on failure
 */
int32_t expand_atom_clauses( patom_t pat );

/**
 * Add a clause to the atoms list
 * @param pat Pointer to the atom
 * @param clause Clause to add.
 * @return 0 onsuccess -1 on failure
 */
int32_t add_clause_to_atom( patom_t pat, uint32_t clause );

/**
 * Learn a new clause based on the current decision tree
 * @param pmt Pointer to the atoms and clauses structures
 * @return returns 0 on success -1 on failure
 */
int32_t learn_clause( pmeta_t pmt, pnds ds );

int32_t check_solved( pmeta_t pmt );

int32_t is_clause_true( pmeta_t pmt, uint32_t clause );

int32_t is_conflict( pmeta_t pmt, uint32_t clause);

int32_t is_set( pmeta_t pmt, uint32_t atom);

int32_t get_val( pmeta_t pmt, uint32_t atom);

int build_graph( pmeta_t pmt, pdl dlist);

void print_decisions( pmeta_t pmt, pclause_t pct, pcl clist, pdl dlist);

/**
 * Add an implied variable 
 * @param ivs Implied variable stack
 * @param atom Atom chosen
 * @param val Atom value -1 for false, 1 for true
 * @return returns 0 on failure, > 0 level otherwise
 */
int add_implied_variable( pimplied_vars_stack ivs, uint32_t atom, int32_t val );

/**
 * Add a decision level
 * @param ds Pointer to the decision stack
 * @param atom Atom chosen
 * @param val Atom value -1 for false, 1 for true
 * @return returns 0 on failure, > 0 level otherwise
 */
int add_decision_level( pnds ds, uint32_t atom, int32_t val );

/**
 * Check if current decision has gone both ways
 * @param ds Pointer to decision stack
 * @return Returns 0 if only true or false attempted 1 if both
 */
int dl_tried_both( pnds ds );

void eval_clause( pmeta_t pmt, pclause_t clause );
#endif
