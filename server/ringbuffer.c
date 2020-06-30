#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"

int dime_ringbuffer_init(dime_ringbuffer_t *ring) {
    ring->cap = 1024;
    ring->arr = malloc(ring->cap);
    if (ring->arr == NULL) {
        return -1;
    }

    ring->len = 0;
    ring->begin = ring->end = 0;

    return 0;
}

void dime_ringbuffer_destroy(dime_ringbuffer_t *ring) {
    free(ring->arr);
}

size_t dime_ringbuffer_read(dime_ringbuffer_t *ring, void *buf, size_t siz) {
    return dime_ringbuffer_discard(ring, dime_ringbuffer_peek(ring, buf, siz));
}

ssize_t dime_ringbuffer_write(dime_ringbuffer_t *ring, const void *buf, size_t siz) {
    if (siz == 0) {
        return 0;
    }

    if (ring->len + siz >= ring->cap) {
        size_t ncap = (3 * (ring->len + siz)) / 2;

        unsigned char *narr = realloc(ring->arr, ncap);
        if (narr == NULL) {
            return -1;
        }

        if (ring->end < ring->begin) {
            size_t nbegin = ring->begin + (ncap - ring->cap);

            memmove(narr + nbegin, narr + ring->begin, ncap - nbegin);

            ring->begin = nbegin;
        }

        ring->arr = narr;
        ring->cap = ncap;
    }

    size_t spaceleft = ring->cap - ring->end;

	if (spaceleft < siz) {
		memcpy(ring->arr + ring->end, buf, spaceleft);
		memcpy(ring->arr, (unsigned char *)buf + spaceleft, siz - spaceleft);
	} else {
		memcpy(ring->arr + ring->end, buf, siz);
	}

    ring->len += siz;
	ring->end += siz;

    if (ring->end >= ring->cap) {
        ring->end -= ring->cap;
    }

    return siz;
}

size_t dime_ringbuffer_peek(const dime_ringbuffer_t *ring, void *buf, size_t siz) {
	if (siz > ring->len) {
		siz = ring->len;
	}

    if (siz == 0) {
        return 0;
    }

	size_t spaceleft = ring->cap - ring->begin;

	if (spaceleft < siz) {
		memcpy(buf, ring->arr + ring->begin, spaceleft);
		memcpy((unsigned char *)buf + spaceleft, ring->arr, siz - spaceleft);
	} else {
		memcpy(buf, ring->arr + ring->begin, siz);
	}

    return siz;
}

size_t dime_ringbuffer_discard(dime_ringbuffer_t *ring, size_t siz) {
	if (siz > ring->len) {
		siz = ring->len;
	}

	ring->len -= siz;
	ring->begin += siz;

    if (ring->begin >= ring->cap) {
        ring->begin -= ring->cap;
    }

    return siz;
}

size_t dime_ringbuffer_len(const dime_ringbuffer_t *ring) {
    return ring->len;
}
