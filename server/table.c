#include <stdlib.h>
#include "table.h"

enum dime_table_elem_use {
    ELEM_FREE,
    ELEM_OCCUPIED,
    ELEM_REMOVED
};

struct dime_table_elem {
    int use;
    uint64_t hash;

    const void *key;
    void *val;
};

struct dime_table {
    size_t ct;

    struct dime_table_elem *arr;

    int (*cmp_f)(const void *, const void *);
    uint64_t (*hash_f)(const void *);

    size_t cap;
    size_t collisions;
    size_t nfree;
};

/* Maximum ratio of used buckets to total buckets; 1 / 2 */
static const double MAX_LOAD_FACTOR = 0.5;

/* Maximum ratio of hash collisions to used buckets; φ / 8 */
static const double MAX_OVERFLOW_FACTOR = 0.20225424859373685602;

static void dime_table_relocate(dime_table_t *tb, size_t i0) {
    size_t i, k;
    struct dime_table_elem tmp;

    if (tb->arr[i0].use != ELEM_OCCUPIED) {
        tb->arr[i0].use = ELEM_FREE;
        return;
    }

    i = tb->arr[i0].hash & (tb->cap - 1);
    k = 1;

    if (i == i0) {
        return;
    }

    tmp = tb->arr[i0];
    tb->arr[i0].use = ELEM_FREE;

    while (dime_table_relocate(tb, i), tb->arr[i].use == ELEM_OCCUPIED) {
        i = (i + k) & (tb->cap - 1);
        k++;
    }

    tb->arr[i] = tmp;

    if (k > 1) {
        tb->collisions++;
    }
}

static int dime_table_grow(dime_table_t *tb) {
    struct dime_table_elem *arr;
    size_t i, cap;

    cap = tb->cap;
    arr = realloc(tb->arr, (cap << 1) * sizeof(struct dime_table_elem));

    if (arr == NULL) {
        return 0;
    }

    tb->arr = arr;
    tb->cap <<= 1;

    tb->collisions = 0;
    tb->nfree = tb->cap - tb->ct;

    for (i = cap; i < tb->cap; i++) {
        tb->arr[i].use = ELEM_FREE;
    }

    for (i = cap; i > 0; i--) {
        dime_table_relocate(tb, i - 1);
    }

    return 1;
}

dime_table_t *dime_table_new(int (*cmp_f)(const void *, const void *), uint64_t (*hash_f)(const void *)) {
    dime_table_t *tb;
    size_t i;

    tb = malloc(sizeof(dime_table_t));
    if (tb == NULL) {
        return NULL;
    }

    tb->cap = 32;
    tb->arr = malloc(tb->cap * sizeof(struct dime_table_elem));
    if (tb->arr == NULL) {
        free(tb);
        return NULL;
    }

    tb->ct = 0;
    tb->cmp_f = cmp_f;
    tb->hash_f = hash_f;

    for (i = 0; i < tb->cap; i++) {
        tb->arr[i].use = ELEM_FREE;
    }

    tb->collisions = 0;
    tb->nfree = tb->cap;

    return tb;
}

void dime_table_free(dime_table_t *tb) {
    free(tb->arr);
    free(tb);
}

int dime_table_insert(dime_table_t *tb, const void *key, void *val) {
    size_t i, k;
    uint64_t hash;
    int prevuse;

    if (tb->ct >= tb->cap) {
        if (!dime_table_grow(tb)) {
            return -1;
        }
    } else if ((double)tb->ct > MAX_LOAD_FACTOR * tb->cap) {
        dime_table_grow(tb);
    }

    hash = tb->hash_f(key);

    i = hash & (tb->cap - 1);
    k = 1;

    while (tb->arr[i].use == ELEM_OCCUPIED) {
        if (tb->cmp_f(key, tb->arr[i].key) == 0) {
            return -1;
        }

        i = (i + k) & (tb->cap - 1);
        k++;
    }

    prevuse = tb->arr[i].use;

    tb->arr[i].use = ELEM_OCCUPIED;
    tb->arr[i].hash = hash;
    tb->arr[i].key = key;
    tb->arr[i].val = val;

    tb->ct++;

    if (k > 1) {
        tb->collisions++;

        if ((double)tb->collisions > MAX_OVERFLOW_FACTOR * tb->ct) {
            dime_table_grow(tb);
        }
    }

    if (prevuse == ELEM_FREE) {
        tb->nfree--;

        if ((double)tb->nfree < MAX_LOAD_FACTOR * tb->cap) {
            tb->nfree = tb->cap - tb->ct;

            for (i = 0; i < tb->cap; i++) {
                dime_table_relocate(tb, i);
            }
        }
    }

    return 0;
}

void *dime_table_search(dime_table_t *tb, const void *key) {
    int cmp;
    size_t i, k;
    struct dime_table_elem *first, *firstavail;

    k = 1;
    firstavail = NULL;

    i = tb->hash_f(key) & (tb->cap - 1);
    first = &tb->arr[i];

    while (tb->arr[i].use != ELEM_FREE) {
        if (tb->arr[i].use == ELEM_OCCUPIED) {
            cmp = tb->cmp_f(key, tb->arr[i].key);

            if (cmp == 0) {
                if (firstavail != NULL) {
                    *firstavail = tb->arr[i];
                    tb->arr[i].use = ELEM_REMOVED;

                    if (first == firstavail) {
                        tb->collisions--;
                    }

                    return firstavail->val;
                } else {
                    return tb->arr[i].val;
                }
            }
        } else if (firstavail == NULL) {
            firstavail = &tb->arr[i];
        }

        i = (i + k) % tb->cap;
        k++;
    }

    return NULL;
}

void *dime_table_remove(dime_table_t *tb, const void *key) {
    int cmp;
    size_t i, k;

    i = tb->hash_f(key) & (tb->cap - 1);
    k = 1;

    while (tb->arr[i].use != ELEM_FREE) {
        if (tb->arr[i].use == ELEM_OCCUPIED) {
            cmp = tb->cmp_f(key, tb->arr[i].key);

            if (cmp == 0) {
                tb->arr[i].use = ELEM_REMOVED;
                tb->ct--;

                return tb->arr[i].val;
            }
        }

        i = (i + k) & (tb->cap - 1);
        k++;
    }

    return NULL;
}

size_t dime_table_size(const dime_table_t *tb) {
    return tb->ct;
}

void dime_table_foreach(dime_table_t *tb, int(*f)(const void *, void *, void *), void *p) {
    size_t i;

    for (i = 0; i < tb->cap; i++) {
        if (tb->arr[i].use == ELEM_OCCUPIED && !f(tb->arr[i].key, tb->arr[i].val, p)) {
            break;
        }
    }
}
