#include "shared.h"

void *xmalloc(size_t size) {
	void *ptr = malloc(size);

	// If a zero size is given, `malloc` is allowed to return `NULL`.
	if (size != 0 && ptr == NULL) {
		fprintf(stderr, "allocation error, unable to allocate to %zu bytes", size);
		abort();
	}

	return ptr;
}

void *xrealloc(void *ptr, size_t size) {
	ptr = realloc(ptr, size);

	// If a zero size is given, `realloc` is allowed to return `NULL`.
	if (size != 0 && ptr == NULL) {
		fprintf(stderr, "allocation error, unable to allocate to %zu bytes", size);
		abort();
	}

	return ptr;
}
