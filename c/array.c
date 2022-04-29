#include "array.h"
#include "shared.h"
#include "value.h"
#include <assert.h>

array *new_array(value *elements, unsigned length, unsigned capacity) {
	array *ary = xmalloc(sizeof(array));

	ary->refcount = 0;
	ary->length = length;
	ary->capacity = capacity;
	ary->elements = elements;

	return ary;
}

void dealloc_array(array *ary) {
	assert(ary->refcount == 0);

	for (unsigned i = 0; i < ary->length; ++i)
		free_value(ary->elements[i]);

	free(ary->elements);
	free(ary);
}

void push_array(array *ary, value val) {
	if (ary->capacity == ary->length) {
		ary->capacity = ary->capacity * 2 + 1;
		ary->elements = xrealloc(ary->elements, ary->capacity * sizeof(value));
	}

	ary->elements[ary->length++] = val;
}

value index_array(const array *ary, int idx) {
	if (idx < 0)
		die("negative indexing isnt supported rn");

	return idx < ary->length ? ary->elements[idx] : VUNDEF;
}

void index_assign_array(array *ary, int idx, value val) {
	if (idx < 0)
		die("negative indexing isnt supported rn");

	// Assigning out of bounds just fills it with `null`.
	while (ary->length <= idx)
		push_array(ary, VNULL);

	ary->elements[idx] = val;
}
