#include "shared.h"

void *xmalloc(size_t size) {
	void *ptr = malloc(size);

	if (size != 0 && ptr == NULL)
		die("allocation error, unable to allocate to %zu bytes", size);

	return ptr;
}

void *xrealloc(void *ptr, size_t size) {
	void *retptr = realloc(ptr, size);

	if (ptr != NULL && size != 0 && retptr == NULL)
		die("allocation error, unable to reallocate to %zu bytes", size);

	return retptr;
}