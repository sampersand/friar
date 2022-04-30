#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "ast.h"

void dump_value(FILE *out, value val) {
	switch (classify(val)) {
	case VK_FUNCTION: {
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

	case VK_ARRAY: {
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

	case VK_BOOLEAN:
		fprintf(out, "Boolean(%s)", as_boolean(val) ? "true" : "false");
		break;

	case VK_NULL:
		fputs("Null()", out);
		break;

	case VK_STRING:
		fprintf(out, "String(%s)", as_string(val)->ptr);
		break;

	case VK_NUMBER:
		fprintf(out, "Number(%lld)", as_number(val));
		break;
	}
}

void free_value(value val) {
	switch (classify(val)) {
	case VK_STRING:
		free_string(as_string(val));
		return;

	case VK_ARRAY:
		free_array(as_array(val));
		return;

	case VK_FUNCTION:
		free_function(as_function(val));
		return;

	default:
		return;
	}
}

value clone_value(value val) {
	switch (classify(val)) {
	case VK_STRING:
		return new_string_value(clone_string(as_string(val)));

	case VK_ARRAY:
		return new_array_value(clone_array(as_array(val)));

	case VK_FUNCTION:
		return new_function_value(clone_function(as_function(val)));

	default:
		return val;
	}
}


const char *value_kind_name(value_kind kind) {
	switch (kind) {
	case VK_BOOLEAN: return "boolean";
	case VK_NULL: return "null";
	case VK_STRING: return "string";
	case VK_FUNCTION: return "function";
	case VK_ARRAY: return "array";
	case VK_NUMBER: return "number";
	}
}

value call_value(value val, unsigned argc, value *argv, environment *env) {
	if (!is_function(val))
		die("cannot call a value of kind %s", value_name(val));

	return call_function(as_function(val), argc, argv, env);
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
	case VK_STRING:;
		string *str = index_string(as_string(val), num_idx);

		if (str == NULL) {
			die("index %lld is out of bounds for string length %u",
				num_idx, as_string(val)->length);
		}

		return new_string_value(str);

	case VK_ARRAY:;
		value ret = index_array(as_array(val), num_idx);

		if (ret == VUNDEF) {
			die("index %lld is out of bounds for array length %u",
				num_idx, as_array(val)->length);
		}

		return ret;

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

	return new_number_value(!as_boolean(val));
}

string *to_string(value val) {
	switch (classify(val)) {
	case VK_STRING:
		return clone_string(as_string(val));

	case VK_NULL:
		return new_string2(strdup("null"), 4);

	case VK_BOOLEAN:
		if (val == VTRUE)
			return new_string2(strdup("true"), 4);
		else
			return new_string2(strdup("false"), 5);

	case VK_NUMBER: {
		// lol there must be a better way to do this
		string *str = alloc_string(47); // length of LONG_LONG_MIN
		sprintf(str->ptr, "%lld", as_number(val));
		str->length = strlen(str->ptr);
		return str;
	}

	default:
		die("cannot no conversion to string defined for %s", value_name(val));
	}
}

value add_values(value lhs, value rhs) {
	// add strings together
	if (is_string(lhs) || is_string(rhs))
		goto string;

	if (classify(lhs) != classify(rhs))
		die("can only add like kinds together, or strings to other types, not %s to %s",
			value_name(lhs), value_name(rhs));

	switch (classify(lhs)) {
	case VK_NUMBER:
		return new_number_value(as_number(lhs) + as_number(rhs));

	case VK_ARRAY:
		return new_array_value(add_arrays(as_array(lhs), as_array(rhs)));

	case VK_STRING:
	string: {
		string *l = to_string(lhs);
		string *r = to_string(rhs);
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
		die("can only multiply numbers, strings, and arrays by numbers, not %s",
			value_name(rhs));
	}

	number amnt = as_number(rhs);

	switch (classify(lhs)) {
	case VK_NUMBER:
		return new_number_value(as_number(lhs) + amnt);

	case VK_STRING:
		return new_string_value(replicate_string(as_string(lhs), amnt));

	case VK_ARRAY:
		return new_string_value(replicate_array(as_array(lhs), amnt));

	default:
		die("can only multiply numbers, strings, and arrays.");
	}
}

value divide_values(value lhs, value rhs) {
	if (!is_number(lhs) || !is_number(rhs))
		die("can only divide numbers from numbers");

	return new_number_value(as_number(lhs) / as_number(rhs));
}

value modulo_values(value lhs, value rhs) {
	if (!is_number(lhs) || !is_number(rhs))
		die("can only modulo numbers with numbers");

	return new_number_value(as_number(lhs) % as_number(rhs));
}

int compare_values(value lhs, value rhs) {
	if (classify(lhs) != classify(rhs))
		die("can only compare like kinds together, not %s to %s",
			value_name(lhs), value_name(rhs));

	switch (classify(lhs)) {
	case VK_NUMBER:
		return compare_numbers(as_number(lhs), as_number(rhs));

	case VK_ARRAY:
		return compare_arrays(as_array(lhs), as_array(rhs));

	case VK_STRING:
		return compare_strings(as_string(lhs), as_string(rhs));

	default:
		die("can only compare numbers, arrays, and strings, not %s", value_name(lhs));
	}
}

bool equate_values(value lhs, value rhs) {
	if (classify(lhs) != classify(rhs))
		return false;

	switch (classify(lhs)) {
	case VK_BOOLEAN:
	case VK_NULL:
	case VK_NUMBER:
	case VK_FUNCTION:
		return lhs == rhs;

	case VK_STRING:
		return !strcmp(as_string(lhs)->ptr, as_string(rhs)->ptr);

	case VK_ARRAY:
		return equate_arrays(as_array(lhs), as_array(rhs));
	}
}
