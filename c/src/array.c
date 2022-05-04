#include <assert.h>

#include "array.h"
#include "shared.h"
#include "value.h"

array *new_array(value *elements, unsigned length, unsigned capacity) {
	array *ary = xmalloc(sizeof(array));

	ary->refcount = 1;
	ary->length = length;
	ary->capacity = capacity;
	ary->elements = elements;

	return ary;
}

void deallocate_array(array *ary) {
	assert(ary->refcount == 0);

	for (unsigned i = 0; i < ary->length; i++)
		free_value(ary->elements[i]);

	free(ary->elements);
	free(ary);
}

void push_array(array *ary, value val) {
	if (ary->capacity == ary->length) {
		ary->capacity = ary->capacity * 2 + 1;
		ary->elements = xrealloc(ary->elements, ary->capacity * sizeof(value));
	}

	ary->elements[ary->length] = val;
	ary->length++;
}

value pop_array(array *ary) {
	if (ary->length == 0)
		return VUNDEF;

	value ret = ary->elements[ary->length];
	ary->length--;
	return ret;
}

value index_array(const array *ary, int idx) {
	if (idx < 0) {
		idx += ary->length;

		if (idx < 0)
			return VUNDEF;
	}

	return (unsigned) idx < ary->length ? ary->elements[idx] : VUNDEF;
}

void index_assign_array(array *ary, int idx, value val) {
	if (idx < 0) {
		idx += ary->length;

		if (idx < 0)
			edie("cannot assign to negative indicies larger than `ary`'s length: %d", idx);
	}


	// Assigning out of bounds just fills it with `null`.
	while (ary->length <= (unsigned) idx)
		push_array(ary, VNULL);

	ary->elements[idx] = val;
}

array *add_arrays(array *lhs, array *rhs) {
	if (lhs->length == 0) return clone_array(rhs);
	if (rhs->length == 0) return clone_array(lhs);

	array *ret = allocate_array(lhs->length + rhs->length);

	for (unsigned i = 0; i < lhs->length; i++)
		ret->elements[i] = clone_value	(lhs->elements[i]);

	for (unsigned i = 0; i < rhs->length; i++)
		ret->elements[i + lhs->length] = clone_value	(rhs->elements[i]);

	ret->length = lhs->length + rhs->length;
	return ret;
}

int compare_arrays(const array *lhs, const array *rhs) {
	unsigned min = lhs->length < rhs->length ? lhs->length : rhs->length;

	for (unsigned i = 0; i < min; i++) {
		int cmp = compare_values(lhs->elements[i], rhs->elements[i]);

		if (cmp != 0)
			return cmp;
	}

	return compare_numbers(lhs->length, rhs->length);
}

bool equate_arrays(const array *lhs, const array *rhs) {
	// short circuit for identical arrays.
	if (lhs == rhs)
		return true;

	if (lhs->length != rhs->length)
		return false;

	for (unsigned i = 0; i < lhs->length; i++) {
		if (!equate_values(lhs->elements[i], rhs->elements[i]))
			return false;
	}

	return true;
}

array *replicate_array(array *ary, unsigned amnt) {
	if (amnt == 1)
		return clone_array(ary);

	array *ret = allocate_array(ary->length * amnt);
	ret->length = 0;

	for (unsigned i = 0; i < amnt; i++) {
		for (unsigned j = 0; j < ary->length; j++) {
			ret->elements[ret->length] = clone_value(ary->elements[j]);
			ret->length++;
		}
	}

	return ret;
}

value delete_at_array(array *ary, int idx) {
	if (idx < 0)  {
		idx += ary->length;

		if (idx < 0)
			return VUNDEF;
	}

	if (ary->length <= (unsigned) idx)
		return VUNDEF;

	value deleted = ary->elements[idx];
	// shift everything over.
	memmove(
		ary->elements + idx,
		ary->elements + idx + 1,
		(ary->length - idx - 1) * sizeof(value)
	);
	ary->length--;

	return deleted;
}

void insert_at_array(array *ary, int idx, value val) {
	if (idx < 0) {
		idx += ary->length;

		if (idx < 0)
			edie("cannot insert to negative indicies larger than `ary`'s length: %d", idx);
	}


	if (ary->length <= (unsigned) idx) {
		index_assign_array(ary, idx, val);
		return;
	}

	if (ary->capacity == ary->length) {
		ary->capacity = ary->capacity * 2 + 1;
		ary->elements = xrealloc(ary->elements, ary->capacity * sizeof(value));
	}

	for (unsigned i = ary->length; i > (unsigned) idx; i--)
		ary->elements[i] = ary->elements[i - 1];

	ary->elements[idx] = val;
	ary->length++;
}

string *array_to_string(const array *ary) {
	if (ary->length == 0)
		return new_string2(strdup("[]"), 2);

	unsigned length = 1; // as we start with `[`.
	unsigned capacity = 8;
	char *str = xmalloc(capacity);
	str[0] = '[';

	// NOTE: this doesn't handle strings well. That'd require a separate "repr" funciton, which I
	// didn't make.
	for (unsigned i = 0; i < ary->length; i++) {
		string *element_string = value_to_string(ary->elements[i]);
		if (capacity <= length + element_string->length + 2) {
			capacity = (length + element_string->length + 2) * 2;
			str = xrealloc(str, capacity);
		}

		if (i != 0) {
			str[length] = ',';
			str[length + 1] = ' ';
			length += 2;
		}

		memcpy(str + length, element_string->ptr, element_string->length);
		length += element_string->length;
		free_string(element_string);
	}

	if (capacity <= length + 2)
		str = xrealloc(str, capacity + 2);
	str[length] = ']';
	str[length + 1] = '\0';
	return new_string2(str, length + 1);
}
