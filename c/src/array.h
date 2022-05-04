#pragma once

#include <assert.h>
#include <stdbool.h>
#include "shared.h"
#include "valuedefn.h"
#include "string.h"
#include <stdio.h>

// The array type within friar.
typedef struct {
	VALUE_ALIGNMENT value *elements;
	unsigned refcount, length, capacity;
} array;

// Creates a new array with the given elements, length, and capacity.
array *new_array(value *elements, unsigned length, unsigned capacity);

// Allocates an array which can old at least `capacity` elements.
static inline array *allocate_array(unsigned capacity) {
	return new_array(xmalloc(sizeof(value) * capacity), 0, capacity);
}

// Deallocates `ary`'s memory. Should only be called when `ary->refcount` is 0.
void deallocate_array(array *ary);

// Stops using `ary`, possibly deallocating it if the refcount is zero.
static inline void free_array(array *ary) {
	assert(ary->refcount != 0); // This means it should have been freed already.

	ary->refcount--;
	if (ary->refcount == 0)
		deallocate_array(ary);
}

// Increments `ary`'s refcount, returning `ary` as a convenience.
static inline array *clone_array(array *ary) {
	ary->refcount++;
	return ary;
}

// Pushes `val` onto the end of `ary`.
void push_array(array *ary, value val);

// Removes the last element from `ary`, returning `VALUE_UNDEFINED` if `ary` is empty.
value pop_array(array *ary);

// Gets the element at `idx` within `ary`, returning `VALUE_UNDEFINED` if out of bounds.
value index_array(const array *ary, int idx);
void index_assign_array(array *ary, int idx, value val);
value delete_at_array(array *ary, int idx);
void insert_at_array(array *ary, int idx, value val);

array *add_arrays(array *lhs, array *rhs);
int compare_arrays(const array *lhs, const array *rhs);
bool equate_arrays(const array *lhs, const array *rhs);
array *replicate_array(array *ary, unsigned amnt);

string *array_to_string(const array *ary);
void dump_array(FILE *out, const array *ary);
