#pragma once
#include <stdio.h>
#include <stdlib.h>

static void *(*__malloc)(size_t) = malloc;
static void *(*__realloc)(void *, size_t) = realloc;

#define malloc(x) (printf("malloc'ing %zu bytes at %s:%d\n", (size_t)x, __FILE__, __LINE__), __malloc(x))
#define realloc(p, x) (printf("realloc'ing %zu bytes at %s:%d\n", (size_t)x, __FILE__, __LINE__), __realloc(p, x))
