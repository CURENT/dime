/*
 * table.h - Hash table
 * Copyright (c) 2020 Nicholas West, Hantao Cui, CURENT, et. al.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * This software is provided "as is" and the author disclaims all
 * warranties with regard to this software including all implied warranties
 * of merchantability and fitness. In no event shall the author be liable
 * for any special, direct, indirect, or consequential damages or any
 * damages whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action, arising
 * out of or in connection with the use or performance of this software.
 */

/**
 * @file table.h
 * @brief Hash table
 * @author Nicholas West
 * @date 2020
 */

#include <stddef.h>
#include <stdint.h>

#ifndef __DIME_table_H
#define __DIME_table_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int use; /** Internal use of this element */

    uint64_t hash;   /** Memoized hash of the key */
    const void *key; /** Key */
    void *val;       /** Value */
} dime_table_elem_t;

typedef struct {
    size_t len; /** Number of elements in the hash table */

    int (*cmp_f)(const void *, const void *); /** Comparison function */
    uint64_t (*hash_f)(const void *);         /** Hashing function */

    dime_table_elem_t *arr; /** Array of elements */
    size_t cap;             /** Capacity of the array */
    size_t ncollisions;     /** Number of collisions */
    size_t nfree;           /** Number of free (not removed) elements */
} dime_table_t;

/**
 * @brief Initialize a new table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param cmp_f Pointer to a comparison function for the keys
 * @param hash_f Pointer to a hashing function for the keys
 *
 * @return A nonnegative value on success, or a negative value on failure
 */
int dime_table_init(dime_table_t *tbl,
                    int (*cmp_f)(const void *, const void *),
                    uint64_t (*hash_f)(const void *));

/**
 * @brief Free resources used by a table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 */
void dime_table_destroy(dime_table_t *tbl);

/**
 * @brief Insert a key-value pair into the table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param key Key
 * @param val Value
 *
 * @return A nonnegative value on success, or a negative value on failure
 */
int dime_table_insert(dime_table_t *tbl, const void *key, void *val);

/**
 * @brief Search for a value in the table via its key
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param key Key
 *
 * @return A pointer to the value on success, or @c NULL on failure
 */
void *dime_table_search(dime_table_t *tbl, const void *key);

/**
 * @brief Remove a value in the table via its key
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param key Key
 *
 * @return A pointer to the value on success, or @c NULL on failure
 */
void *dime_table_remove(dime_table_t *tbl, const void *key);

/**
 * @brief Get the number of elements in the table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 *
 * @return Number of elements in the table
 */
size_t dime_table_len(const dime_table_t *tbl);

typedef struct {
    const void *key; /** Key */
    void *val;       /** Value */

    dime_table_t *tbl; /** Table */
    size_t i;          /** Index in table */
} dime_table_iter_t;

/**
 * @brief Initialize a new table iterator
 *
 * Initialize a new iterator for the table @em tbl. The member variables @c key and @c val point to the current key-value pair the iterator is on. Two important caveats for the iterators created via this function:
 *
 * - The iterator need not be explicitly destroyed; it does not use any heap memory on its own
 *
 *
 * - @link dime_table_iter_next @endlink must be called at least once before @c key and @c val point to a valid key-value pair. This is similar to how Python's iterators behave with @c iter and @c next.
 *
 *
 * Below is a code example showing how to use the iterators:
@code
dime_table_iter_t it;
dime_table_iter_init(&it, pointer_to_my_table);

while (dime_table_iter_next(&it)) {
    do_something_with(it.key, it.val);
}
@endcode
 *
 *
 */
void dime_table_iter_init(dime_table_iter_t *it, dime_table_t *tbl);

int dime_table_iter_next(dime_table_iter_t *it);

/**
 * @brief Apply a function for each key-value pair in the table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param f Function to apply
 * @param p Pointer passed as third argument to @em f
 */
void dime_table_apply(dime_table_t *tbl,
                      int(*f)(const void *, void *, void *),
                      void *p);

#ifdef __cplusplus
}
#endif

#endif
