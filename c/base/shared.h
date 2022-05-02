#pragma once
#include <stdio.h>
#include <stdlib.h>

#define die(...) (fprintf(stderr, __VA_ARGS__), fputs("\n", stderr), exit(1))

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

#ifndef NDEBUG
# define LOG(...) (LOGN(__VA_ARGS__), puts(""))
# define LOGN(...) (printf("%s:%d ", __FILE__, __LINE__), printf(__VA_ARGS__))
#else
# define LOG(...) ((void) 0)
# define LOGN(...) ((void) 0)
#endif
