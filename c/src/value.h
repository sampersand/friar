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

/*
As Friar is a dynamically typed langauge, all possible types are encoded within
a single `value` type. To differentiate between types, we use encode type data
via bit masking: The bottom three bits are used to distinguish the different
types of values. This takes advantage of the fact that all the pointer types
(i.e. `string`, `function`, `array`, and `builtin_function`) are all 8 aligned,
which means the bottom three bits are used for storing information.

As such, `number` is really a 61 bit integer, as three bits are used for tagging.

The scheme is laid out as follows:
000...000 = VFALSE
000...001 = VNULL
000...010 = VTRUE
000...011 = VUNDEF (indicates "undefined")
XXX...000 = string
XXX...001 = user-defined function
XXX...010 = ary
XXX...011 = builtin function
XXX...100 = number
*/
enum {
	VALUE_TAG_STRING           = 0, 
	VALUE_TAG_FUNCTION         = 1,
	VALUE_TAG_ARRAY            = 2,
	VALUE_TAG_BUILTIN_FUNCTION = 3,
	VALUE_TAG_NUMBER           = 4,
	VALUE_TAG_MASK             = 7,
};

/*
 * An enum used to indicate what type a `value` is.
 *
 * Note that this is separate from the `VALUE_TAG_`s, as booleans and null
 * dont actually have a tag.
 */
typedef enum {
	VALUE_KIND_BOOLEAN,
	VALUE_KIND_NULL,
	VALUE_KIND_STRING,
	VALUE_KIND_FUNCTION,
	VALUE_KIND_BUILTIN_FUNCTION,
	VALUE_KIND_ARRAY,
	VALUE_KIND_NUMBER,
} value_kind;

// Returns a string representation of `kind`.
const char *value_kind_name(value_kind kind);

// Returns the kind of value `val` is.
static inline value_kind classify(value val) {
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
	default: bug("invalid value: %lld (%llx)", val & VALUE_TAG_MASK, val);
	}
}

// Returns a string representation of `val`'s kind.
static inline const char *value_name(value val) {
	return value_kind_name(classify(val));
}

// Creates a new `value` out of a `bool`.
static inline value new_boolean_value(bool boolean) {
	return boolean ? VTRUE : VFALSE;
}

// Creates a new `value` out of an `array`.
static inline value new_array_value(array *ary) {
	assert(((value) ary & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) ary | VALUE_TAG_ARRAY;
}

// Creates a new `value` out of a `number`.
static inline value new_number_value(number num) {
	return ((value) num << 3) | VALUE_TAG_NUMBER;
}

// Creates a new `value` out of a `string`.
static inline value new_string_value(string *str) {
	assert(((value) str & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) str | VALUE_TAG_STRING;
}

// Creates a new `value` out of a `function`.
static inline value new_function_value(function *func) {
	assert(((value) func & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) func | VALUE_TAG_FUNCTION;
}

// Creates a new `value` out of a `builtin_function`.
static inline value new_builtin_function_value(builtin_function *builtin_func) {
	assert(((value) builtin_func & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) builtin_func | VALUE_TAG_BUILTIN_FUNCTION;
}

// Checks if `val` is null.
static inline bool is_null(value val) {
	return val == VNULL;
}

// Checks if `val` is a `bool`.
static inline bool is_boolean(value val) {
	return val == VTRUE || val == VFALSE;
}

// Checks if `val` is an `array`.
static inline bool is_array(value val) {
	return classify(val) == VALUE_KIND_ARRAY;
}

// Checks if `val` is a `number`.
static inline bool is_number(value val) {
	return classify(val) == VALUE_KIND_NUMBER;
}

// Checks if `val` is a `string`.
static inline bool is_string(value val) {
	return classify(val) == VALUE_KIND_STRING;
}

// Checks if `val` is a `function`.
static inline bool is_function(value val) {
	return classify(val) == VALUE_KIND_FUNCTION;
}

// Checks if `val` is a `builtin_function`.
static inline bool is_builtin_function(value val) {
	return classify(val) == VALUE_KIND_BUILTIN_FUNCTION;
}

// Casts `val` to a `bool` without verifying its type.
static inline bool as_boolean(value val) {
	assert(is_boolean(val));
	return val == VTRUE;
}

// Casts `val` to a `number` without verifying its type.
static inline number as_number(value val) {
	assert(is_number(val));
	return (number) val >> 3;
}

// Casts `val` to an `array` without verifying its type.
static inline array *as_array(value val) {
	assert(is_array(val));
	return (array *) (val & ~VALUE_TAG_MASK);
}

// Casts `val` to a `string` without verifying its type.
static inline string *as_string(value val) {
	assert(is_string(val));
	return (string *) (val & ~VALUE_TAG_MASK);
}

// Casts `val` to a `function` without verifying its type.
static inline function *as_function(value val) {
	assert(is_function(val));
	return (function *) (val & ~VALUE_TAG_MASK);
}

// Casts `val` to a `builtin_function` without verifying its type.
static inline builtin_function *as_builtin_function(value val) {
	assert(is_builtin_function(val));
	return (builtin_function *) (val & ~VALUE_TAG_MASK);
}

// Dumps a debug representation of `val` to `out`.
void dump_value(FILE *out, value val);

// Frees `val`; as `val`s have reference-counting semantics, use `free_value`
// when you're done with a value.
void free_value(value val);

// Clones `val`; as `val`s have reference-counting semantics, use `clone_value`
// when you need to duplicate ownership.
value clone_value(value val);

// Converts `val` to a string.
string *value_to_string(value val);

// Calls `val` with the given arguments and `environment`.
value call_value(
	value val,
	unsigned number_of_arguments,
	value *arguments,
	environment *env
);

// Numerically negates `val`.
value negate_value(value val, const environment *env);

// Logically negates `val`.
value not_value(value val, const environment *env);

// Adds `lhs` to `rhs`.
value add_values(value lhs, value rhs, const environment *env);

// Subtracts `rhs` from `lhs`.
value subtract_values(value lhs, value rhs, const environment *env);

// Multiplies `lhs` with `rhs`.
value multiply_values(value lhs, value rhs, const environment *env);

// Divides `lhs` by `rhs`.
value divide_values(value lhs, value rhs, const environment *env);

// Modulos `lhs` by `rhs`.
value modulo_values(value lhs, value rhs, const environment *env);

// Returns true if `lhs` equals `rhs`.
bool equate_values(value lhs, value rhs, const environment *env);

// Returns a negative, zero, or positive number depending on if `lhs` is 
// less than, equal to, or greater than `rhs`.
int compare_values(value lhs, value rhs, const environment *env);

// Gets the element at `idx` within `source`.
value index_value(value source, value idx, const environment *env);

// Assigns the element at `idx` to `val` within `source`.
void index_assign_value(value source, value idx, value val, const environment *env);
