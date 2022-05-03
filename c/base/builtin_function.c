#include "base/builtin_function.h"
#include "base/value.h"

value call_builtin_function(
	builtin_function *builtin_func,
	unsigned number_of_arguments,
	const value *arguments,
	environment *env
) {
	if (builtin_func->required_argument_count != number_of_arguments) {
		die("argument mismatch, %s expected %d, but got %d",
			builtin_func->name, builtin_func->required_argument_count, number_of_arguments);
	}

	return (builtin_func->function_pointer)(arguments, env);
}

static value builtin_to_str_fn(const value *arguments, environment *env) {
	(void) env;
	return new_string_value(value_to_string(arguments[0]));
}

static value builtin_to_num_fn(const value *arguments, environment *env) {
	(void) arguments;
	(void) env;
	return VNULL;
}

static value builtin_prompt_fn(const value *arguments, environment *env) {
	(void) arguments;
	(void) env;
	return VNULL;
}

static value builtin_print_fn(const value *arguments, environment *env) {
	(void) env;
	string *to_print;

	if (is_string(arguments[0])) {
		to_print = clone_string(as_string(arguments[0]));
	} else {
		to_print = value_to_string(arguments[0]);
	}

	printf("%s", to_print->ptr);
	fflush(stdout);
	free_string(to_print);
	return VNULL;
}

static value builtin_println_fn(const value *arguments, environment *env) {
	builtin_print_fn(arguments, env);
	puts("");
	return VNULL;
}

static value builtin_random_fn(const value *arguments, environment *env) {
	(void) arguments;
	(void) env;
	return VNULL;
}

static value builtin_length_fn(const value *arguments, environment *env) {
	(void) env;

	switch (classify(arguments[0])) {
	case VALUE_KIND_ARRAY:
		return new_number_value(as_array(arguments[0])->length);

	case VALUE_KIND_STRING:
		return new_number_value(as_string(arguments[0])->length);

	default:
		die("can only get the length of arrays and strings, not %s", value_name(arguments[0]));
	}
}

static value builtin_exit_fn(const value *arguments, environment *env) {
	(void) env;
	if (!is_number(arguments[0]))
		die("can only exit with an integer status code, not %s", value_name(arguments[0]));

	exit(as_number(arguments[0]));
}


builtin_function builtin_functions[] = {
	{
		.name = "to_str",
		.required_argument_count = 1,
		.function_pointer = builtin_to_str_fn
	},
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
};

