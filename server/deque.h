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
 */

#include <stddef.h>

#ifndef __DIME_deque_H
#define __DIME_deque_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t len; /** Number of elements in the deque */

    void **arr; /** Array of elements */
    size_t cap; /** Capacity of array */

    size_t begin; /** Index of head of the deque in the array */
    size_t end;   /** One plus index of the tail of the deque in the array */
} dime_deque_t;

/**
 * @brief Initialize a new deque
 *
 * @param deck Pointer to a @c dime_deque_t struct
 *
 * @return A nonnegative value on success, or a negative value on failure
 */
int dime_deque_init(dime_deque_t *deck);

/**
 * @brief Free resources used by a deque
 *
 * @param deck Pointer to a @c dime_deque_t struct
 */
void dime_deque_destroy(dime_deque_t *deck);

/**
 * @brief Prepend an element to the deque
 *
 * @param deck Pointer to a @c dime_deque_t struct
 * @param p Element
 *
 * @return A nonnegative value on success, or a negative value on failure
 */
int dime_deque_pushl(dime_deque_t *deck, void *p);

/**
 * @brief Append an element to the deque
 *
 * @param deck Pointer to a @c dime_deque_t struct
 * @param p Element
 *
 * @return A nonnegative value on success, or a negative value on failure
 */
int dime_deque_pushr(dime_deque_t *deck, void *p);

/**
 * @brief Pop an element from the head of the deque
 *
 * @param deck Pointer to a @c dime_deque_t struct
 *
 * @return The element at the head of the deque, or NULL if the deque is empty
 */
void *dime_deque_popl(dime_deque_t *deck);

/**
 * @brief Pop an element from the tail of the deque
 *
 * @param deck Pointer to a @c dime_deque_t struct
 *
 * @return The element at the tail of the deque, or NULL if the deque is empty
 */
void *dime_deque_popr(dime_deque_t *deck);

typedef struct {
    void *val;

    dime_deque_t *deck;
    size_t i;
} dime_deque_iter_t;

void dime_deque_iter_init(dime_deque_iter_t *it, dime_deque_t *deck);

int dime_deque_iter_next(dime_deque_iter_t *it);

/**
 * @brief Apply a function for each element in the deque
 *
 * @param deck Pointer to a @c dime_deque_t struct
 * @param f Function to apply
 * @param p Pointer passed as second argument to @em f
 */
void dime_deque_apply(dime_deque_t *deck, int(*f)(void *, void *), void *p);

/**
 * @brief Get the number of elements in the deque
 *
 * @param tbl Pointer to a @c dime_deque_t struct
 *
 * @return Number of elements in the deque
 */
size_t dime_deque_len(dime_deque_t *deck);

#ifdef __cplusplus
}
#endif

#endif
