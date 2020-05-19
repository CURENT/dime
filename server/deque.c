#include <stdlib.h>
#include <string.h>

#include "deque.h"

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

void dime_deque_foreach(dime_deque_t *deck, int(*f)(void *, void *), void *p) {
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

size_t dime_deque_len(dime_deque_t *deck) {
    return deck->len;
}
