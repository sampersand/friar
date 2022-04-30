#pragma once

#include <assert.h>
#include <stdbool.h>
#include "shared.h"
#include "valuedefn.h"

typedef struct {
	unsigned refcount, length, capacity;
	value *elements;
} array;

array *new_array3(value *elements, unsigned length, unsigned capacity);

static inline array *new_array2(value *elements, unsigned length) {
	return new_array3(elements, length, length);
}

static inline array *allocate_array(unsigned capacity) {
	value *elements = xmalloc(sizeof(value) * capacity);
	return new_array3(elements, 0, capacity);
}

void deallocate_array(array *ary);

static inline void free_array(array *ary) {
#ifndef WE_SOLVED_FREE_ISSUES
	return;
#endif

	assert(ary->refcount != 0);

	ary->refcount--;
	if (ary->refcount == 0)
		deallocate_array(ary);
}

static inline array *clone_array(array *ary) {
	ary->refcount++;
	return ary;
}

void push_array(array *ary, value val);
value index_array(const array *ary, int idx);
void index_assign_array(array *ary, int idx, value val);
array *add_arrays(array *lhs, array *rhs);
int compare_arrays(const array *lhs, const array *rhs);
bool equate_arrays(const array *lhs, const array *rhs);
array *replicate_array(array *ary, unsigned amnt);
