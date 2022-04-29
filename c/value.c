#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "ast.h"

void dump_value(FILE *out, value val) {
	switch (classify(val)) {
	case VK_BOOLEAN:
		fprintf(out, "Boolean(%s)", as_boolean(val) ? "true" : "false");
		break;

	case VK_NULL:
		fputs("Null()", out);
		break;

	case VK_STRING:
		fprintf(out, "String(%s)", as_string(val)->ptr);
		break;

	case VK_FUNCTION: {
		function *func = as_function(val);
		fprintf(out, "Function(%s, args=[", func->name);

		for (unsigned i = 0; i < func->argc; ++i) {
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
		for (unsigned i = 0; i < ary->length; ++i) {
			if (i != 0)
				fputs(", ", out);

			dump_value(out, ary->elements[i]);
		}
		fputc(')', out);

		break;
	}

	case VK_NUMBER:
		fprintf(out, "Number(%lld)", as_number(val));
		break;
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

value call_value(value val, int argc, value *argv, environment *env) {
	if (!is_function(val))
		die("cannot call a value of kind %s", value_kind_name(classify(val)));

	return call_function(as_function(val), argc, argv, env);
}

void index_assign(value ary, value idx, value val) {
	if (!is_array(ary))
		die("can only index assign into arrays");

	if (!is_number(idx))
		die("you must index with numbers");

	index_assign_array(as_array(ary), as_number(idx), val);
}

value index_into(value ary, value idx) {
	if (!is_number(idx))
		die("you must index with numbers");

	number num_idx = as_number(idx);

	switch (classify(ary)) {
	case VK_STRING:;
		string *str = index_string(as_string(ary), num_idx);
		return str == NULL ? VNULL : new_string_value(str);

	case VK_ARRAY:;
		value ret = index_array(as_array(ary), num_idx);
		return ret == VUNDEF ? VNULL : ret;

	default:
		die("can only index into arrays or strs");
	}
}
