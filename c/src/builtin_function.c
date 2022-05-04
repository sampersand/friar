#include "builtin_function.h"
#include "value.h"

value call_builtin_function(
	builtin_function *builtin,
	unsigned number_of_arguments,
	const value *arguments
) {
	if (builtin->required_argument_count != number_of_arguments) {
		edie("argument mismatch, %s expected %d, but got %d",
			builtin->name, builtin->required_argument_count, number_of_arguments);
	}

	return (builtin->function_pointer)(arguments);
}

void dump_builtin_function(FILE *out, const builtin_function *builtin) {
	fprintf(out, "BuiltinFunction(%s)\n", builtin->name);
}

static value builtin_to_num_fn(const value *arguments) {
	die("[todo] to num");
	(void) arguments;
	return VNULL;
}

static value builtin_prompt_fn(const value *arguments) {
	die("[todo] prompt");
	(void) arguments;
	return VNULL;
}

static value builtin_print_fn(const value *arguments) {
	string *to_print;

	if (is_string(arguments[0])) {
		to_print = clone_string(as_string(arguments[0]));
	} else {
		to_print = value_to_string(arguments[0]);
	}

	printf("%.*s", to_print->length, to_print->ptr);
	fflush(stdout);
	free_string(to_print);
	return VNULL;
}

static value builtin_println_fn(const value *arguments) {
	builtin_print_fn(arguments);
	puts("");
	return VNULL;
}

static value builtin_random_fn(const value *arguments) {
	(void) arguments;
	die("[todo] random fn");
	return VNULL;
}

static value builtin_length_fn(const value *arguments) {
	switch (classify(arguments[0])) {
	case VALUE_KIND_ARRAY:
		return new_number_value(as_array(arguments[0])->length);

	case VALUE_KIND_STRING:
		return new_number_value(as_string(arguments[0])->length);

	default:
		edie("can only get the length of arrays and strings, not %s", value_name(arguments[0]));
	}
}

static value builtin_exit_fn(const value *arguments) {
	if (!is_number(arguments[0]))
		edie("can only exit with an integer status code, not %s", value_name(arguments[0]));

	exit(as_number(arguments[0]));
}

static value builtin_dump_fn(const value *arguments) {
	dump_value(stdout, arguments[0]);
	putc('\n', stdout);
	return arguments[0];
}

static value builtin_delete_fn(const value *arguments) {
	if (!is_array(arguments[0]))
		edie("can only `delete` from arrays, not %s", value_name(arguments[0]));

	if (!is_number(arguments[1]))
		edie("index needs to be an integer for `delete`, not %s", value_name(arguments[1]));

	value ret = delete_at_array(as_array(arguments[0]), as_number(arguments[1]));
	return ret == VUNDEF ? VNULL : ret;
}

static value builtin_insert_fn(const value *arguments) {
	if (!is_array(arguments[0]))
		edie("can only `insert` from arrays, not %s", value_name(arguments[0]));

	if (!is_number(arguments[1]))
		edie("index needs to be an integer for `insert`, not %s", value_name(arguments[1]));

	insert_at_array(as_array(arguments[0]), as_number(arguments[1]), arguments[2]);
	return arguments[0];
}

builtin_function builtin_functions[] = {
	{
		.name = "to_num",
		.required_argument_count = 1,
		.function_pointer = builtin_to_num_fn
	},
	{
		.name = "prompt",
		.required_argument_count = 0,
		.function_pointer = builtin_prompt_fn
	},
	{
		.name = "print",
		.required_argument_count = 1,
		.function_pointer = builtin_print_fn
	},
	{
		.name = "println",
		.required_argument_count = 1,
		.function_pointer = builtin_println_fn
	},
	{
		.name = "random",
		.required_argument_count = 0,
		.function_pointer = builtin_random_fn
	},
	{
		.name = "length",
		.required_argument_count = 1,
		.function_pointer = builtin_length_fn
	},
	{
		.name = "exit",
		.required_argument_count = 1,
		.function_pointer = builtin_exit_fn
	},
	{
		.name = "dump",
		.required_argument_count = 1,
		.function_pointer = builtin_dump_fn
	},
	{
		.name = "delete",
		.required_argument_count = 2,
		.function_pointer = builtin_delete_fn
	},
	{
		.name = "insert",
		.required_argument_count = 3,
		.function_pointer = builtin_insert_fn
	},
};

