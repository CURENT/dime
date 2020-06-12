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
 *
 * Implements an associative array via a hash table with quadratic
 * probing and lazy deletion. Allows for @f$\mathcal{O}(1)@f$ average
 * time on insertions, lookups, and removals. In particular, the
 * internal array size is always a power of two, and the probing
 * sequence is based on the triangular numbers (positive integers of the
 * form @f$ \sum_{i = 1}^{n} i @f$ or @f$ {n(n + 1)} \over 2 @f$).
 * Unlike most other quadratic probing methods, this assures that every
 * index is eventually visited when probing for a certain element. Even
 * so, the implementation maintains a maximum load factor of ½ and a
 * maximum ratio of bucket collisions to total elements of @f$ \varphi
 * \over 8 @f$ to ensure optimal performance.
 */

#include <stddef.h>
#include <stdint.h>

#ifndef __DIME_table_H
#define __DIME_table_H

#ifdef __cplusplus
extern "C" {
#endif

/* Actual element in the array of the hash table */
typedef struct {
    int use; /* Internal use of this element */

    uint64_t hash;   /* Memoized hash of the key */
    const void *key; /* Key */
    void *val;       /* Value */
} dime_table_elem_t;

/**
 * @brief Hash table
 *
 * Should be treated as opaque; use relevant methods to access elements
 * in the table.
 *
 * @see dime_table_init
 * @see dime_table_destroy
 * @see dime_table_insert
 * @see dime_table_search
 * @see dime_table_search_const
 * @see dime_table_remove
 * @see dime_table_len
 * @see dime_table_iter_t
 */
typedef struct {
    size_t len; /* Number of elements in the hash table */

    int (*cmp_f)(const void *, const void *); /* Comparison function */
    uint64_t (*hash_f)(const void *);         /* Hashing function */

    dime_table_elem_t *arr; /* Array of elements */
    size_t cap;             /* Capacity of the array */
    size_t ncollisions;     /* Number of collisions */
    size_t nfree;           /* Number of free (not removed) elements */
} dime_table_t;

/**
 * @brief Initialize a new table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param cmp_f Pointer to a comparison function for the keys
 * @param hash_f Pointer to a hashing function for the keys
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_table_destroy
 */
int dime_table_init(dime_table_t *tbl,
                    int (*cmp_f)(const void *, const void *),
                    uint64_t (*hash_f)(const void *));

/**
 * @brief Free resources used by a table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 *
 * @see dime_table_init
 */
void dime_table_destroy(dime_table_t *tbl);

/**
 * @brief Insert a key-value pair into the table
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param key Key
 * @param val Value
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_table_search
 * @see dime_table_remove
 */
int dime_table_insert(dime_table_t *tbl, const void *key, void *val);

/**
 * @brief Search for a value in the table via its key
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param key Key
 *
 * @return A pointer to the value on success, or @c NULL on failure
 *
 * @see dime_table_search_const
 * @see dime_table_insert
 * @see dime_table_remove
 */
void *dime_table_search(dime_table_t *tbl, const void *key);

/**
 * @brief Search for a value in the table via its key (const-safe)
 *
 * Similar to @link dime_table_search @endlink, but is guaranteed to not
 * mutate the underlying table. This allows for safe concurrent searches
 * of read-only tables, but because no element relocation is performed
 * for lazy deletions, subsequent searches for a table that has had many
 * insertions/removals may be slower. If your table is not exposed to
 * more than one thread, then @link dime_table_search @endlink should be
 * preferred.
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param key Key
 *
 * @return A pointer to the value on success, or @c NULL on failure
 *
 * @see dime_table_search
 */
const void *dime_table_search_const(const dime_table_t *tbl, const void *key);

/**
 * @brief Remove a value in the table via its key
 *
 * @param tbl Pointer to a @c dime_table_t struct
 * @param key Key
 *
 * @return A pointer to the value on success, or @c NULL on failure
 *
 * @see dime_table_insert
 * @see dime_table_search
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

/**
 * @brief Table iterator
 *
 * Iterator for a hash table. Contains at least the following member
 * variables:
 * - @code const void *key; @endcode
 *   The key of the element at the current iterator position.
 * - @code void *val; @endcode
 *   The value of the element at the current iterator position.
 *
 * Two important caveats for iterators:
 * - After initializing the iterator with @link dime_table_iter_init
 *   @endlink, @link dime_table_iter_next @endlink must be called at
 *   least once before @c key and @c val point to a valid key-value pair
 *   in the table. This is similar to how Python's iterators behave with
 *   @c iter and @c next.
 * - The iterator need not be explicitly destroyed; it does not use any
 *   heap memory on its own.
 *
 * Below is a code example showing how to use the iterators:
@code
dime_table_iter_t it;
dime_table_iter_init(&it, pointer_to_my_table);

while (dime_table_iter_next(&it)) {
    do_something_with(it.key, it.val);
}
@endcode

 * @see dime_table_iter_init
 * @see dime_table_iter_next
 * @see dime_table_apply
 * @see dime_table_t
 */

typedef struct {
    const void *key; /** Key of current element */
    void *val;       /** Value of current element */

    dime_table_t *tbl; /* Table */
    size_t i;          /* Index in table array */
} dime_table_iter_t;

/**
 * @brief Initialize a table iterator
 *
 * @param it Pointer to a @link dime_table_iter_t @endlink struct
 * @param tbl Pointer to a @link dime_table_t @endlink struct
 */
void dime_table_iter_init(dime_table_iter_t *it, dime_table_t *tbl);

/**
 * @brief Get the next element from the table iterator
 *
 * @param it Pointer to a @link dime_table_iter_t @endlink struct
 *
 * @return A nonzero value if the iterator successfully advanced in the
 * table, or zero if the iterator has reached the end of the table
 */
int dime_table_iter_next(dime_table_iter_t *it);

/**
 * @brief Execute a function for each key-value pair in the table
 *
 * @em f should return a non-zero value to continue, and a zero value to
 * break. Equivalent to the following code snippet:
@code
dime_table_iter_t it;
dime_table_iter_init(&it, tbl);

while (dime_table_iter_next(&it)) {
    if (!f(it.key, it.val, p)) {
        break;
    }
}
@endcode
 *
 * @param deck Pointer to a @c dime_table_t struct
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
