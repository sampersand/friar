#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "valuedefn.h"
#include "shared.h"
#include "string.h"
#include "array.h"
#include "number.h"
#include "function.h"
#include "environment.h"
#include "ast.h"

typedef enum {
	VK_BOOLEAN,
	VK_NULL,
	VK_STRING,
	VK_FUNCTION,
	VK_ARRAY,
	VK_NUMBER,
} value_kind;

enum {
	TAG_STRING   = 0,
	TAG_FUNCTION = 1,
	TAG_ARRAY    = 2,
	TAG_NUMBER   = 4,
	TAG_MASK     = (TAG_STRING | TAG_FUNCTION | TAG_ARRAY | TAG_NUMBER)
};

static inline value_kind classify(value val) {
	if (val == VNULL)
		return VK_NULL;

	if (val == VTRUE || val == VFALSE)
		return VK_BOOLEAN;

	switch (val & TAG_MASK) {
	case TAG_STRING: return VK_STRING;
	case TAG_FUNCTION: return VK_FUNCTION;
	case TAG_ARRAY: return VK_ARRAY;
	case TAG_NUMBER: return VK_NUMBER;
	default: die("[bug] invalid value tag: %lld", val & TAG_MASK);
	}
}

const char *value_kind_name(value_kind kind);

static inline const char *value_name(value val) {
	return value_kind_name(classify(val));
}

static inline value new_array_value(array *ary) {
	assert(((value) ary & TAG_MASK) == 0);

	return (value) ary | TAG_ARRAY;
}

static inline value new_number_value(number num) {
	return ((value) num << 3) | TAG_NUMBER;
}

static inline value new_string_value(string *str) {
	assert(((value) str & TAG_MASK) == 0);
	return (value) str | TAG_STRING;
}

static inline value new_function_value(function *func) {
	assert(((value) func & TAG_MASK) == 0);
	return (value) func | TAG_FUNCTION;
}

static inline value new_boolean_value(bool boolean) {
	return boolean ? VTRUE : VFALSE;
}

static inline bool is_boolean(value val) {
	return val == VTRUE || val == VFALSE;
}

static inline bool is_null(value val) {
	return val == VTRUE;
}

static inline bool is_array(value val) {
	return classify(val) == VK_ARRAY;
}

static inline bool is_number(value val) {
	return classify(val) == VK_NUMBER;
}

static inline bool is_string(value val) {
	return classify(val) == VK_STRING;
}

static inline bool is_function(value val) {
	return classify(val) == VK_FUNCTION;
}

static inline bool as_boolean(value val) {
	assert(is_boolean(val));
	return val == VTRUE;
}

static inline array *as_array(value val) {
	assert(is_array(val));
	return (array *) (val & ~TAG_MASK);
}

static inline number as_number(value val) {
	assert(is_number(val));
	return (number) val >> 3;
}

static inline string *as_string(value val) {
	assert(is_string(val));
	return (string *) (val & ~TAG_MASK);
}

static inline function *as_function(value val) {
	assert(is_function(val));
	return (function *) (val & ~TAG_MASK);
}

void dump_value(FILE *out, value val);

void index_assign_value(value ary, value idx, value val);
value index_value(value ary, value idx);
value call_value(value val, unsigned argc, value *argv, environment *env);

value negate_value(value val);
value not_value(value val);
value add_values(value lhs, value rhs);
value subtract_values(value lhs, value rhs);
value multiply_values(value lhs, value rhs);
value divide_values(value lhs, value rhs);
value modulo_values(value lhs, value rhs);
int compare_values(value lhs, value rhs);
bool equate_values(value lhs, value rhs);

void free_value(value val);
value clone_value(value val);

string *value_to_string(value val);
