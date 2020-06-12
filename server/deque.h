/*
 * deque.h - Double-ended queue
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
 * @file deque.h
 * @brief Double-ended queue
 * @author Nicholas West
 * @date 2020
 *
 * Implements a double-ended queue (or deque) via a dynamic ring buffer.
 * Deques allow for @f$\mathcal{O}(1)@f$ amortized insertions and
 * removals from both their left and right ends.
 */

#include <stddef.h>

#ifndef __DIME_deque_H
#define __DIME_deque_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Double-ended queue
 *
 * Should be treated as opaque; use relevant methods to access elements
 * in the deque.
 *
 * @see dime_deque_init
 * @see dime_deque_destroy
 * @see dime_deque_pushl
 * @see dime_deque_pushr
 * @see dime_deque_popl
 * @see dime_deque_popr
 * @see dime_deque_len
 * @see dime_deque_iter_t
 */
typedef struct {
    size_t len; /* Number of elements in the deque */

    void **arr; /* Array of elements */
    size_t cap; /* Capacity of array */

    size_t begin; /* Index of head of the deque in the array */
    size_t end;   /* One plus index of the tail of the deque in the array */
} dime_deque_t;

/**
 * @brief Initialize a new deque
 *
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_deque_destroy
 */
int dime_deque_init(dime_deque_t *deck);

/**
 * @brief Free resources used by a deque
 *
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 *
 * @see dime_deque_init
 */
void dime_deque_destroy(dime_deque_t *deck);

/**
 * @brief Prepend an element to the deque
 *
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 * @param p Element
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_deque_pushr
 * @see dime_deque_popl
 */
int dime_deque_pushl(dime_deque_t *deck, void *p);

/**
 * @brief Append an element to the deque
 *
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 * @param p Element
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_deque_pushl
 * @see dime_deque_popr
 */
int dime_deque_pushr(dime_deque_t *deck, void *p);

/**
 * @brief Pop an element from the head of the deque
 *
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 *
 * @return The element at the head of the deque, or NULL if the deque is
 * empty
 *
 * @see dime_deque_popr
 * @see dime_deque_pushl
 */
void *dime_deque_popl(dime_deque_t *deck);

/**
 * @brief Pop an element from the tail of the deque
 *
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 *
 * @return The element at the tail of the deque, or NULL if the deque is
 * empty
 *
 * @see dime_deque_popr
 * @see dime_deque_pushl
 */
void *dime_deque_popr(dime_deque_t *deck);

/**
 * @brief Get the number of elements in the deque
 *
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 *
 * @return Number of elements in the deque
 */
size_t dime_deque_len(const dime_deque_t *deck);

/**
 * @brief Deque iterator
 *
 * Iterator for a double-ended queue. Contains at least the following
 * member variables:
 * - @code void *val; @endcode
 *   The value of the element at the current iterator position.
 *
 * Two important caveats for iterators:
 * - After initializing the iterator with @link dime_deque_iter_init
 *   @endlink, @link dime_deque_iter_next @endlink must be called at
 *   least once before @c val points to a valid element in the deque.
 *   This is similar to how Python's iterators behave with @c iter and
 *   @c next.
 * - The iterator need not be explicitly destroyed; it does not use any
 *   heap memory on its own.
 *
 * Below is a code example showing how to use the iterators:
@code
dime_deque_iter_t it;
dime_deque_iter_init(&it, pointer_to_my_deque);

while (dime_deque_iter_next(&it)) {
    do_something_with(it.val);
}
@endcode
 *
 * @see dime_deque_iter_init
 * @see dime_deque_iter_next
 * @see dime_deque_apply
 * @see dime_deque_t
 */
typedef struct {
    void *val; /** Current element */

    dime_deque_t *deck; /* Deque */
    size_t i;           /* Index in deque array */
} dime_deque_iter_t;

/**
 * @brief Initialize a deque iterator
 *
 * @param it Pointer to a @link dime_deque_iter_t @endlink struct
 * @param deck Pointer to a @link dime_deque_t @endlink struct
 */
void dime_deque_iter_init(dime_deque_iter_t *it, dime_deque_t *deck);

/**
 * @brief Get the next element from the deque iterator
 *
 * @param it Pointer to a @link dime_deque_iter_t @endlink struct
 *
 * @return A nonzero value if the iterator successfully advanced in the
 * deque, or zero if the iterator has reached the end of the deque
 */
int dime_deque_iter_next(dime_deque_iter_t *it);

/**
 * @brief Execute a function for each element in the deque
 *
 * @em f should return a non-zero value to continue, and a zero value to
 * break. Equivalent to the following code snippet:
 *
@code
dime_deque_iter_t it;
dime_deque_iter_init(&it, deck);

while (dime_deque_iter_next(&it)) {
    if (!f(it.val, p)) {
        break;
    }
}
@endcode
 *
 * @param deck Pointer to a @c dime_deque_t struct
 * @param f Function to apply
 * @param p Pointer passed as second argument to @em f
 */
void dime_deque_apply(dime_deque_t *deck, int(*f)(void *, void *), void *p);

#ifdef __cplusplus
}
#endif

#endif
