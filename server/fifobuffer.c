#include <stdlib.h>
#include <string.h>

#include "fifobuffer.h"

struct dime_fifobuffer {
    size_t ct;

    unsigned char *arr;
    size_t cap;
    size_t begin, end;
};

dime_fifobuffer_t *dime_fifobuffer_new() {
    dime_fifobuffer_t *fifo = malloc(sizeof(dime_fifobuffer_t));
    if (fifo == NULL) {
        return NULL;
    }

    fifo->cap = 1024;
    fifo->arr = malloc(fifo->cap);
    if (fifo->arr == NULL) {
        free(fifo);
        return NULL;
    }

    fifo->ct = 0;
    fifo->begin = fifo->end = 0;

    return fifo;
}

void dime_fifobuffer_free(dime_fifobuffer_t *fifo) {
    free(fifo->arr);
    free(fifo);
}

size_t dime_fifobuffer_read(dime_fifobuffer_t *fifo, void *p, size_t siz) {
    return dime_fifobuffer_discard(fifo, dime_fifobuffer_peek(fifo, p, siz));
}

ssize_t dime_fifobuffer_write(dime_fifobuffer_t *fifo, const void *p, size_t siz) {
    if (siz == 0) {
        return 0;
    }

    if (fifo->ct + siz >= fifo->cap) {
        size_t ncap = (3 * (fifo->ct + siz)) / 2;

        unsigned char *narr = realloc(fifo->arr, ncap);
        if (narr == NULL) {
            return -1;
        }

        if (fifo->end < fifo->begin) {
            size_t nbegin = fifo->begin + (ncap - fifo->cap);

            memmove(narr + nbegin, narr + fifo->begin, ncap - nbegin);

            fifo->begin = nbegin;
        }

        fifo->arr = narr;
        fifo->cap = ncap;
    }

    size_t spaceleft = fifo->cap - fifo->end;

	if (spaceleft < siz) {
		memcpy(fifo->arr + fifo->end, p, spaceleft);
		memcpy(fifo->arr, (unsigned char *)p + spaceleft, siz - spaceleft);
	} else {
		memcpy(fifo->arr + fifo->end, p, siz);
	}

    fifo->ct += siz;
	fifo->end += siz;

    if (fifo->end >= fifo->cap) {
        fifo->end -= fifo->cap;
    }

    return siz;
}

size_t dime_fifobuffer_peek(const dime_fifobuffer_t *fifo, void *p, size_t siz) {
	if (siz > fifo->ct) {
		siz = fifo->ct;
	}

    if (siz == 0) {
        return 0;
    }

	size_t spaceleft = fifo->cap - fifo->begin;

	if (spaceleft < siz) {
		memcpy(p, fifo->arr + fifo->begin, spaceleft);
		memcpy((unsigned char *)p + spaceleft, fifo->arr, siz - spaceleft);
	} else {
		memcpy(p, fifo->arr + fifo->begin, siz);
	}

    return siz;
}

size_t dime_fifobuffer_discard(dime_fifobuffer_t *fifo, size_t siz) {
	if (siz > fifo->ct) {
		siz = fifo->ct;
	}

	fifo->ct -= siz;
	fifo->begin += siz;

    if (fifo->begin >= fifo->cap) {
        fifo->begin -= fifo->cap;
    }

    return siz;
}

size_t dime_fifobuffer_size(const dime_fifobuffer_t *fifo) {
    return fifo->ct;
}
