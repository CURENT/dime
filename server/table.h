/*
 * table.h - Hash table
 * Copyright (c) 2020 Nicholas West, CURENT, et. al.
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
 * @brief Implements a hash table
 * @author Nicholas West
 * @date 2020
 *
 * A hash table is one method for implementing an associative array, also known as a map or dictionary, an abstract container type that allows for random access of a set of keys. Associative arrays allow for three general operations: insertion of a new key-value pair, removal of a key-value pair based on its key, and search/modification of a value based on its key. Because these operations can be generalized to a plethora of uses, such as databases, object instances, and mathematical sets, associative arrays are among the most versatile container types in computing.
 *
 * Compared to @link tree.h binary search trees @endlink , hash tables have several advantages and drawbacks, the most significant one being that hash tables have a significantly better asymptotic performance of @f$ O(1) @f$ amortized time for all operations, as opposed to the best case of @f$ O(\log n) @f$ for a binary search tree. Additionally, hash tables exhibit better spatial locality, resulting in fewer cache misses. However, in the worst case, a hash table can store all of its entries in a single bucket, resulting in @f$ O(n) @f$ time. This is in stark contrast of binary search trees, which, if a self-balancing version is used, keeps its average-case complexity of @f$ O(\log n) @f$ regardless of the input data. That having been said, hash tables turn out to be slightly faster in most practical scenarios, and are the usual choice if the intended usage primarily consists of random access.
 *
 * This implementation of a hash table uses open addressing with quadratic probing and lazy deletion. More specifically, the internal array size is always a power of two, and the probing sequence is based on the triangular numbers, i.e. positive integers of the form @f$ \sum_{i = 1}^{n} i @f$ or @f$ {n(n + 1)} \over 2 @f$. Unlike most other quadratic probing methods, this assures that every index is eventually visited when probing for a certain element. Even so, this implementation maintains a maximum load factor of @f$ 1 \over 2 @f$ and a maximum ratio of bucket collisions to total elements of @f$ \varphi \over 8 @f$ to ensure optimal performance.
 */

#include <stddef.h>
#include <stdint.h>

#ifndef __DIME_table_H
#define __DIME_table_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dime_table dime_table_t;

dime_table_t *dime_table_new(int (*cmp_f)(const void *, const void *), uint64_t (*hash_f)(const void *));

void dime_table_free(dime_table_t *tb);

int dime_table_insert(dime_table_t *tb, const void *key, void *val);

void *dime_table_search(dime_table_t *tb, const void *key);

void *dime_table_remove(dime_table_t *tb, const void *key);

size_t dime_table_size(const dime_table_t *tb);

void dime_table_foreach(dime_table_t *tb, int(*f)(const void *, void *, void *), void *p);

#ifdef __cplusplus
}
#endif

#endif
