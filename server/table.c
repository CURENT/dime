/*
 * table.c - Hash table
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

#include <stdlib.h>

#include "error.h"
#include "table.h"

enum dime_table_elem_use {
    ELEM_FREE,
    ELEM_OCCUPIED,
    ELEM_REMOVED
};

/* Maximum ratio of used buckets to total buckets; 1 / 2 */
static const double MAX_LOAD_FACTOR = 0.5;

/* Maximum ratio of hash collisions to used buckets; φ / 8 */
static const double MAX_OVERFLOW_FACTOR = 0.20225424859373685602;

static void dime_table_relocate(dime_table_t *tbl, size_t i0) {
    size_t i, k;
    dime_table_elem_t tmp;

    if (tbl->arr[i0].use != ELEM_OCCUPIED) {
        tbl->arr[i0].use = ELEM_FREE;
        return;
    }

    i = tbl->arr[i0].hash & (tbl->cap - 1);
    k = 1;

    if (i == i0) {
        return;
    }

    tmp = tbl->arr[i0];
    tbl->arr[i0].use = ELEM_FREE;

    while (dime_table_relocate(tbl, i), tbl->arr[i].use == ELEM_OCCUPIED) {
        i = (i + k) & (tbl->cap - 1);
        k++;
    }

    tbl->arr[i] = tmp;

    if (k > 1) {
        tbl->ncollisions++;
    }
}

static int dime_table_grow(dime_table_t *tbl) {
    dime_table_elem_t *arr;
    size_t cap;

    cap = tbl->cap;
    arr = realloc(tbl->arr, (cap << 1) * sizeof(dime_table_elem_t));

    if (arr == NULL) {
        return 0;
    }

    tbl->arr = arr;
    tbl->cap <<= 1;

    tbl->ncollisions = 0;
    tbl->nfree = tbl->cap - tbl->len;

    for (size_t i = cap; i < tbl->cap; i++) {
        tbl->arr[i].use = ELEM_FREE;
    }

    for (size_t i = cap; i > 0; i--) {
        dime_table_relocate(tbl, i - 1);
    }

    return 1;
}

int dime_table_init(dime_table_t *tbl, int (*cmp_f)(const void *, const void *), uint64_t (*hash_f)(const void *)) {
    tbl->cap = 32;
    tbl->arr = malloc(tbl->cap * sizeof(dime_table_elem_t));
    if (tbl->arr == NULL) {
        dime_errorstring("Out of memory");
        return -1;
    }

    tbl->len = 0;
    tbl->cmp_f = cmp_f;
    tbl->hash_f = hash_f;

    for (size_t i = 0; i < tbl->cap; i++) {
        tbl->arr[i].use = ELEM_FREE;
    }

    tbl->ncollisions = 0;
    tbl->nfree = tbl->cap;

    return 0;
}

void dime_table_destroy(dime_table_t *tbl) {
    free(tbl->arr);
}

int dime_table_insert(dime_table_t *tbl, const void *key, void *val) {
    if (tbl->len >= tbl->cap) {
        if (!dime_table_grow(tbl)) {
            dime_errorstring("Out of memory");
            return -1;
        }
    } else if ((double)tbl->len > MAX_LOAD_FACTOR * tbl->cap) {
        dime_table_grow(tbl);
    }

    size_t i, k;
    uint64_t hash;

    hash = tbl->hash_f(key);

    i = hash & (tbl->cap - 1);
    k = 1;

    while (tbl->arr[i].use == ELEM_OCCUPIED) {
        if (tbl->cmp_f(key, tbl->arr[i].key) == 0) {
            dime_errorstring("Key collision");
            return -1;
        }

        i = (i + k) & (tbl->cap - 1);
        k++;
    }

    int prevuse = tbl->arr[i].use;

    tbl->arr[i].use = ELEM_OCCUPIED;
    tbl->arr[i].hash = hash;
    tbl->arr[i].key = key;
    tbl->arr[i].val = val;

    tbl->len++;

    if (k > 1) {
        tbl->ncollisions++;

        if ((double)tbl->ncollisions > MAX_OVERFLOW_FACTOR * tbl->len) {
            dime_table_grow(tbl);
        }
    }

    if (prevuse == ELEM_FREE) {
        tbl->nfree--;

        if ((double)tbl->nfree < MAX_LOAD_FACTOR * tbl->cap) {
            tbl->nfree = tbl->cap - tbl->len;

            for (i = 0; i < tbl->cap; i++) {
                dime_table_relocate(tbl, i);
            }
        }
    }

    return 0;
}

void *dime_table_search(dime_table_t *tbl, const void *key) {
    size_t i, k, first, firstavail;

    i = tbl->hash_f(key) & (tbl->cap - 1);
    k = 1;

    first = i;
    firstavail = ((size_t)-1);

    while (tbl->arr[i].use != ELEM_FREE) {
        if (tbl->arr[i].use == ELEM_OCCUPIED) {
            int cmp = tbl->cmp_f(key, tbl->arr[i].key);

            if (cmp == 0) {
                if (firstavail != ((size_t)-1)) {
                    tbl->arr[firstavail] = tbl->arr[i];
                    tbl->arr[i].use = ELEM_REMOVED;

                    if (first == firstavail) {
                        tbl->ncollisions--;
                    }

                    i = firstavail;
                }

                return tbl->arr[i].val;
            }
        } else if (firstavail == ((size_t)-1)) {
            firstavail = i;
        }

        i = (i + k) & (tbl->cap - 1);
        k++;
    }

    return NULL;
}

const void *dime_table_search_const(const dime_table_t *tbl, const void *key) {
    size_t i, k;

    i = tbl->hash_f(key) & (tbl->cap - 1);
    k = 1;

    while (tbl->arr[i].use != ELEM_FREE) {
        if (tbl->arr[i].use == ELEM_OCCUPIED) {
            int cmp = tbl->cmp_f(key, tbl->arr[i].key);

            if (cmp == 0) {
                return tbl->arr[i].val;
            }
        }

        i = (i + k) & (tbl->cap - 1);
        k++;
    }

    return NULL;
}

void *dime_table_remove(dime_table_t *tbl, const void *key) {
    size_t i, k;

    i = tbl->hash_f(key) & (tbl->cap - 1);
    k = 1;

    while (tbl->arr[i].use != ELEM_FREE) {
        if (tbl->arr[i].use == ELEM_OCCUPIED) {
            int cmp = tbl->cmp_f(key, tbl->arr[i].key);

            if (cmp == 0) {
                tbl->arr[i].use = ELEM_REMOVED;
                tbl->len--;

                return tbl->arr[i].val;
            }
        }

        i = (i + k) & (tbl->cap - 1);
        k++;
    }

    return NULL;
}

size_t dime_table_len(const dime_table_t *tbl) {
    return tbl->len;
}

void dime_table_iter_init(dime_table_iter_t *it, dime_table_t *tbl) {
    it->tbl = tbl;
    it->i = (size_t)-1;
}

int dime_table_iter_next(dime_table_iter_t *it) {
    do {
        it->i++;
    } while (it->i < it->tbl->cap && it->tbl->arr[it->i].use != ELEM_OCCUPIED);

    if (it->i == it->tbl->cap) {
        return 0;
    }

    it->key = it->tbl->arr[it->i].key;
    it->val = it->tbl->arr[it->i].val;

    return 1;
}

void dime_table_apply(dime_table_t *tbl, int(*f)(const void *, void *, void *), void *p) {
    for (size_t i = 0; i < tbl->cap; i++) {
        if (tbl->arr[i].use == ELEM_OCCUPIED && !f(tbl->arr[i].key, tbl->arr[i].val, p)) {
            break;
        }
    }
}
