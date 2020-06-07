#include <stdlib.h>
#include <string.h>

#include "deque.h"
#include "error.h"

struct dime_deque {
    size_t len;

    void **arr;
    size_t cap;

    size_t begin, end;
};

int dime_deque_init(dime_deque_t *deck) {
    deck->len = 0;

    deck->cap = 16;
    deck->arr = malloc(deck->cap * sizeof(void *));
    if (deck->arr == NULL) {
        dime_errorstring("Out of memory");
        return -1;
    }

    deck->begin = deck->end = 0;

    return 0;
}

void dime_deque_destroy(dime_deque_t *deck) {
    free(deck->arr);
}

int dime_deque_pushl(dime_deque_t *deck, void *p) {
    if (deck->len >= deck->cap) {
        size_t ncap = (3 * deck->cap) / 2;
        void **narr = realloc(deck->arr, ncap * sizeof(void *));
        if (narr == NULL) {
            dime_errorstring("Out of memory");
            return -1;
        }

        if (deck->end <= deck->begin) {
            size_t nbegin = deck->begin + (ncap - deck->cap);

            memmove(narr + nbegin, narr + deck->begin, (ncap - nbegin) * sizeof(void *));

            deck->begin = nbegin;
        }

        deck->arr = narr;
        deck->cap = ncap;
    }

    if (deck->begin == 0) {
        deck->begin = deck->cap;
    }
    deck->begin--;

    deck->arr[deck->begin] = p;

    deck->len++;

    return 0;
}

int dime_deque_pushr(dime_deque_t *deck, void *p) {
    if (deck->len >= deck->cap) {
        size_t ncap = (3 * deck->cap) / 2;
        void **narr = realloc(deck->arr, ncap * sizeof(void *));
        if (narr == NULL) {
            dime_errorstring("Out of memory");
            return -1;
        }

        if (deck->end <= deck->begin) {
            size_t nbegin = deck->begin + (ncap - deck->cap);

            memmove(narr + nbegin, narr + deck->begin, (ncap - nbegin) * sizeof(void *));

            deck->begin = nbegin;
        }

        deck->arr = narr;
        deck->cap = ncap;
    }

    deck->arr[deck->end] = p;

    deck->end++;
    if (deck->end == deck->cap) {
        deck->end = 0;
    }

    deck->len++;

    return 0;
}

void *dime_deque_popl(dime_deque_t *deck) {
    if (deck->len == 0) {
        return NULL;
    }

    void *p = deck->arr[deck->begin];

    deck->begin++;
    if (deck->begin == deck->cap) {
        deck->begin = 0;
    }

    deck->len--;

    return p;
}

void *dime_deque_popr(dime_deque_t *deck) {
    if (deck->len == 0) {
        return NULL;
    }

    if (deck->end == 0) {
        deck->begin = deck->cap;
    }
    deck->end--;

    void *p = deck->arr[deck->end];

    deck->len--;

    return p;
}

size_t dime_deque_len(dime_deque_t *deck) {
    return deck->len;
}

void dime_deque_iter_init(dime_deque_iter_t *it, dime_deque_t *deck) {
    it->deck = deck;
    it->i = deck->begin - 1;
}

int dime_deque_iter_next(dime_deque_iter_t *it) {
    it->i++;

    if (it->i == it->deck->cap) {
        it->i = 0;
    }

    if (it->i == it->deck->end) {
        return 0;
    }

    it->val = it->deck->arr[it->i];

    return 1;
}

void dime_deque_apply(dime_deque_t *deck, int(*f)(void *, void *), void *p) {
    size_t i = deck->begin;

    while (i != deck->end) {
        if (!f(deck->arr[i], p)) {
            break;
        }

        i++;

        if (i == deck->cap) {
            i = 0;
        }
    }
}
