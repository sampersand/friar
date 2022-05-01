#pragma once
#include <stdio.h>
#include <stdlib.h>

#define die(...) (fprintf(stderr, __VA_ARGS__), exit(1))

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
