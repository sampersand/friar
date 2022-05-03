#include <stdlib.h>
#include <string.h>
#include "base/value.h"
#include "base/ast.h"

void dump_value(FILE *out, value val) {
	switch (classify(val)) {
	case VALUE_KIND_BUILTIN_FUNCTION:
		fprintf(out, "BuiltinFunction(%s)\n", as_builtin_function(val)->name);
		break;

	case VALUE_KIND_FUNCTION: {
		function *func = as_function(val);
		fprintf(out, "Function(%s, args=[", func->name);

		for (unsigned i = 0; i < func->argc; i++) {
			if (i != 0)
				fputs(", ", out);

			fputs(func->argv[i], out);
		}

		fputs("])", out);
		break;
	}

	case VALUE_KIND_ARRAY: {
		array *ary = as_array(val);

		fprintf(out, "Array(");
		for (unsigned i = 0; i < ary->length; i++) {
			if (i != 0)
				fputs(", ", out);

			dump_value(out, ary->elements[i]);
		}
		fputc(')', out);

		break;
	}

	case VALUE_KIND_BOOLEAN:
		fprintf(out, "Boolean(%s)", as_boolean(val) ? "true" : "false");
		break;

	case VALUE_KIND_NULL:
		fputs("Null()", out);
		break;

	case VALUE_KIND_STRING:
		fprintf(out, "String(%s)", as_string(val)->ptr);
		break;

	case VALUE_KIND_NUMBER:
		fprintf(out, "Number(%lld)", as_number(val));
		break;
	}
}

void free_value(value val) {
	switch (classify(val)) {
	case VALUE_KIND_STRING:
		free_string(as_string(val));
		return;

	case VALUE_KIND_ARRAY:
		free_array(as_array(val));
		return;

	case VALUE_KIND_FUNCTION:
		free_function(as_function(val));
		return;

	default:
		return;
	}
}

value clone_value(value val) {
	switch (classify(val)) {
	case VALUE_KIND_STRING:
		return new_string_value(clone_string(as_string(val)));

	case VALUE_KIND_ARRAY:
		return new_array_value(clone_array(as_array(val)));

	case VALUE_KIND_FUNCTION:
		return new_function_value(clone_function(as_function(val)));

	default:
		return val;
	}
}


const char *value_kind_name(value_kind kind) {
	switch (kind) {
	case VALUE_KIND_BOOLEAN:          return "boolean";
	case VALUE_KIND_NULL:             return "null";
	case VALUE_KIND_STRING:           return "string";
	case VALUE_KIND_ARRAY:            return "array";
	case VALUE_KIND_NUMBER:           return "number";
	case VALUE_KIND_FUNCTION:         return "function";
	case VALUE_KIND_BUILTIN_FUNCTION: return "function"; // should be indistinguishable
	}
}

value call_value(value val, unsigned argc, value *argv, environment *env) {
	switch (classify(val)) {
	case VALUE_KIND_FUNCTION:
		return call_function(as_function(val), argc, argv, env);

	case VALUE_KIND_BUILTIN_FUNCTION:
		return call_builtin_function(as_builtin_function(val), argc, argv, env);

	default:
		die("cannot call a value of kind %s", value_name(val));
	}
}

void index_assign_value(value ary, value idx, value val) {
	if (!is_array(ary))
		die("can only index assign into arrays");

	if (!is_number(idx))
		die("you must index with numbers");

	index_assign_array(as_array(ary), as_number(idx), val);
}

value index_value(value val, value idx) {
	if (!is_number(idx))
		die("you must index with numbers");

	number num_idx = as_number(idx);

	switch (classify(val)) {
	case VALUE_KIND_STRING: {
		string *str = index_string(as_string(val), num_idx);

		if (str == NULL) {
			die("index %lld is out of bounds for string of length %u",
				num_idx, as_string(val)->length);
		}

		return new_string_value(str);
	}

	case VALUE_KIND_ARRAY: {
		value ret = index_array(as_array(val), num_idx);

		if (ret == VUNDEF) {
			die("index %lld is out of bounds for array of length %u",
				num_idx, as_array(val)->length);
		}

		return ret;
	}

	default:
		die("can only index into arrays or strs");
	}
}

value negate_value(value val) {
	if (!is_number(val))
		die("can only negate numbers, not %s", value_name(val));

	return new_number_value(-as_number(val));
}

value not_value(value val) {
	if (!is_boolean(val))
		die("can only not booleans, not %s", value_name(val));

	return new_boolean_value(!as_boolean(val));
}

string *value_to_string(value val) {
	switch (classify(val)) {
	case VALUE_KIND_STRING:
		return clone_string(as_string(val));

	case VALUE_KIND_NUMBER:
		return number_to_string(as_number(val));

	// `new_string2` requires ownership of its string, so we `strdup`.
	case VALUE_KIND_BOOLEAN:
		if (val == VTRUE) {
			return new_string2(strdup("true"), 4);
		} else {
			return new_string2(strdup("false"), 5);
		}

	case VALUE_KIND_NULL:
		return new_string2(strdup("null"), 4);

	default:
		die("cannot no conversion to string defined for %s", value_name(val));
	}
}

value add_values(value lhs, value rhs) {
	// If either side is a string, we convert both to strings then add.
	if (is_string(lhs) || is_string(rhs))
		goto string;

	if (classify(lhs) != classify(rhs)) {
		die("can only add like kinds together, or strings to other types, not %s to %s",
			value_name(lhs), value_name(rhs));
	}

	switch (classify(lhs)) {
	case VALUE_KIND_NUMBER:
		return new_number_value(as_number(lhs) + as_number(rhs));

	case VALUE_KIND_ARRAY:
		return new_array_value(add_arrays(as_array(lhs), as_array(rhs)));

	case VALUE_KIND_STRING:
	string: {
		string *l = value_to_string(lhs);
		string *r = value_to_string(rhs);
		string *ret = add_strings(l, r);

		free_string(l);
		free_string(r);

		return new_string_value(ret);
	}

	default:
		die("can only add numbers, arrays, and strings, not %s", value_name(lhs));
	}
}

value subtract_values(value lhs, value rhs) {
	if (!is_number(lhs) || !is_number(rhs))
		die("can only subtract numbers from numbers");

	return new_number_value(as_number(lhs) - as_number(rhs));
}

value multiply_values(value lhs, value rhs) {
	if (!is_number(rhs))  {
		die("can only multiply numbers, strings, and arrays by numbers, not %ss",
			value_name(rhs));
	}

	number amnt = as_number(rhs);

	switch (classify(lhs)) {
	case VALUE_KIND_NUMBER:
		return new_number_value(as_number(lhs) * amnt);

	case VALUE_KIND_STRING:
		if (amnt < 0)
			die("can only multiply strings by nonnegative integers.");

		return new_string_value(replicate_string(as_string(lhs), amnt));

	case VALUE_KIND_ARRAY:
		if (amnt < 0)
			die("can only multiply arrays by nonnegative integers.");

		return new_array_value(replicate_array(as_array(lhs), amnt));

	default:
		die("can only multiply numbers, strings, and arrays.");
	}
}

value divide_values(value lhs, value rhs) {
	if (!is_number(lhs) || !is_number(rhs))
		die("can only divide numbers from numbers");

	if (as_number(rhs) == 0)
		die("division by zero!");

	return new_number_value(as_number(lhs) / as_number(rhs));
}

value modulo_values(value lhs, value rhs) {
	if (!is_number(lhs) || !is_number(rhs))
		die("can only modulo numbers with numbers");

	if (as_number(rhs) == 0)
		die("modulo by zero!");

	return new_number_value(as_number(lhs) % as_number(rhs));
}

int compare_values(value lhs, value rhs) {
	if (classify(lhs) != classify(rhs)) {
		die("can only compare like kinds together, not %s to %s",
			value_name(lhs), value_name(rhs));
	}

	switch (classify(lhs)) {
	case VALUE_KIND_NUMBER:
		return compare_numbers(as_number(lhs), as_number(rhs));

	case VALUE_KIND_ARRAY:
		return compare_arrays(as_array(lhs), as_array(rhs));

	case VALUE_KIND_STRING:
		return compare_strings(as_string(lhs), as_string(rhs));

	default:
		die("can only compare numbers, arrays, and strings, not %s", value_name(lhs));
	}
}

bool equate_values(value lhs, value rhs) {
	if (classify(lhs) != classify(rhs))
		return false;

	switch (classify(lhs)) {
	case VALUE_KIND_BOOLEAN:
	case VALUE_KIND_NULL:
	case VALUE_KIND_NUMBER:
	case VALUE_KIND_FUNCTION:
	case VALUE_KIND_BUILTIN_FUNCTION:
		return lhs == rhs;

	case VALUE_KIND_STRING:
		return !strcmp(as_string(lhs)->ptr, as_string(rhs)->ptr);

	case VALUE_KIND_ARRAY:
		return equate_arrays(as_array(lhs), as_array(rhs));
	}
}
