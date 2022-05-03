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
#include "builtin_function.h"
#include "ast.h"

typedef enum {
	VALUE_KIND_BOOLEAN,
	VALUE_KIND_NULL,
	VALUE_KIND_STRING,
	VALUE_KIND_FUNCTION,
	VALUE_KIND_BUILTIN_FUNCTION,
	VALUE_KIND_ARRAY,
	VALUE_KIND_NUMBER,
} value_kind;

enum {
	VALUE_TAG_STRING           = 0,
	VALUE_TAG_FUNCTION         = 1,
	VALUE_TAG_ARRAY            = 2,
	VALUE_TAG_BUILTIN_FUNCTION = 3,
	VALUE_TAG_NUMBER           = 4,
	VALUE_TAG_MASK             = 7,
};

static inline value_kind classify(value val) {
	if (val == VUNDEF)
		strlen(xmalloc(2));
	assert(val != VUNDEF);

	if (val == VNULL)
		return VALUE_KIND_NULL;

	if (val == VTRUE || val == VFALSE)
		return VALUE_KIND_BOOLEAN;

	switch (val & VALUE_TAG_MASK) {
	case VALUE_TAG_STRING:           return VALUE_KIND_STRING;
	case VALUE_TAG_FUNCTION:         return VALUE_KIND_FUNCTION;
	case VALUE_TAG_BUILTIN_FUNCTION: return VALUE_KIND_BUILTIN_FUNCTION;
	case VALUE_TAG_ARRAY:            return VALUE_KIND_ARRAY;
	case VALUE_TAG_NUMBER:           return VALUE_KIND_NUMBER;
	default: die("[bug] invalid value tag: %lld (%llx)", val & VALUE_TAG_MASK, val);
	}
}

const char *value_kind_name(value_kind kind);

static inline const char *value_name(value val) {
	return value_kind_name(classify(val));
}

static inline value new_array_value(array *ary) {
	assert(((value) ary & VALUE_TAG_MASK) == 0);

	return (value) ary | VALUE_TAG_ARRAY;
}

static inline value new_number_value(number num) {
	return ((value) num << 3) | VALUE_TAG_NUMBER;
}

static inline value new_string_value(string *str) {
	assert(((value) str & VALUE_TAG_MASK) == 0);
	return (value) str | VALUE_TAG_STRING;
}

static inline value new_function_value(function *func) {
	assert(((value) func & VALUE_TAG_MASK) == 0);
	return (value) func | VALUE_TAG_FUNCTION;
}

static inline value new_builtin_function_value(builtin_function *builtin_func) {
	assert(((value) builtin_func & VALUE_TAG_MASK) == 0);
	return (value) builtin_func | VALUE_TAG_BUILTIN_FUNCTION;
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
	return classify(val) == VALUE_KIND_ARRAY;
}

static inline bool is_number(value val) {
	return classify(val) == VALUE_KIND_NUMBER;
}

static inline bool is_string(value val) {
	return classify(val) == VALUE_KIND_STRING;
}

static inline bool is_function(value val) {
	return classify(val) == VALUE_KIND_FUNCTION;
}

static inline bool is_builtin_function(value val) {
	return classify(val) == VALUE_KIND_BUILTIN_FUNCTION;
}

static inline bool as_boolean(value val) {
	assert(is_boolean(val));
	return val == VTRUE;
}

static inline array *as_array(value val) {
	assert(is_array(val));
	return (array *) (val & ~VALUE_TAG_MASK);
}

static inline number as_number(value val) {
	assert(is_number(val));
	return (number) val >> 3;
}

static inline string *as_string(value val) {
	assert(is_string(val));
	return (string *) (val & ~VALUE_TAG_MASK);
}

static inline function *as_function(value val) {
	assert(is_function(val));
	return (function *) (val & ~VALUE_TAG_MASK);
}

static inline builtin_function *as_builtin_function(value val) {
	assert(is_builtin_function(val));
	return (builtin_function *) (val & ~VALUE_TAG_MASK);
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
