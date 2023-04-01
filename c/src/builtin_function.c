#include "builtin_function.h"
#include "value/value.h"
#include <time.h>

void init_builtin_functions(void) {
	// the `srandom` function provides much better random numbers, but isn't technically standard.
#ifdef SRANDOM_UNDEFINED
	srand(time(NULL));
#else
	srandom(time(NULL));
#endif
}

value call_builtin_function(
	const builtin_function *builtin_func,
	unsigned number_of_arguments,
	const value *arguments
) {
	if (builtin_func->required_argument_count != number_of_arguments) {
		die_with_stacktrace("argument mismatch, %s expected %d, but got %d",
			builtin_func->name, builtin_func->required_argument_count, number_of_arguments);
	}

	return (builtin_func->function_pointer)(arguments);
}

void dump_builtin_function(FILE *out, const builtin_function *builtin_func) {
	fprintf(out, "BuiltinFunction(%s)\n", builtin_func->name);
}

static value builtin_to_num_fn(const value *arguments) {
	if (!is_string(arguments[0]))
		die_with_stacktrace("Can only convert strings to numbers, not %s", value_name(arguments[0]));

	return new_number_value(string_to_number(as_string(arguments[0])));
}

static value builtin_prompt_fn(const value *arguments) {
	(void) arguments;

	size_t capacity = 0;
	ssize_t length;
	char *line = NULL;

	// TODO: use fgets instead
	if ((length = getline(&line, &capacity, stdin)) == -1) {
		assert(line != NULL);
		free(line);

		if (!feof(stdin))
			die_with_stacktrace("unable to read line from stdin");

		return new_string_value(new_string(strdup(""), 0));
	}

	assert(0 < length);
	assert(line != NULL);

	// strip trialing newlines
	if (length != 0 && line[length - 1] == '\n') {
		length--;
		if (length != 0 && line[length - 1] == '\r')
			length--;
	}

	value ret = new_string_value(new_string(strndup(line, length), length));
	free(line);

	return ret;
}

static value builtin_print_fn(const value *arguments) {
	string *to_print = value_to_string(arguments[0]);

	printf("%.*s", to_print->length, to_print->ptr);
	fflush(stdout);

	free_string(to_print);
	return VALUE_NULL;
}

static value builtin_println_fn(const value *arguments) {
	builtin_print_fn(arguments);

	putchar('\n');

	return VALUE_NULL;
}

static value builtin_random_fn(const value *arguments) {
	(void) arguments;
	number ret;

#ifdef SRANDOM_UNDEFINED
	ret = rand();
#else
	ret = random();
#endif

	return new_number_value(ret);
}

static value builtin_length_fn(const value *arguments) {
	switch (classify(arguments[0])) {
	case VALUE_KIND_ARRAY:
		return new_number_value(as_array(arguments[0])->length);

	case VALUE_KIND_STRING:
		return new_number_value(as_string(arguments[0])->length);

	default:
		die_with_stacktrace("can only get the length of arrays and strings, not %s", value_name(arguments[0]));
	}
}

static value builtin_exit_fn(const value *arguments) {
	if (!is_number(arguments[0]))
		die_with_stacktrace("can only exit with an integer status code, not %s", value_name(arguments[0]));

	exit(as_number(arguments[0]));
}

static value builtin_dump_fn(const value *arguments) {
	dump_value(stdout, arguments[0]);
	putchar('\n');

	return clone_value(arguments[0]);
}

static value builtin_delete_fn(const value *arguments) {
	if (!is_array(arguments[0]))
		die_with_stacktrace("can only `delete` from arrays, not %s", value_name(arguments[0]));

	if (!is_number(arguments[1]))
		die_with_stacktrace("index needs to be an integer for `delete`, not %s", value_name(arguments[1]));

	value ret = delete_at_array(as_array(arguments[0]), as_number(arguments[1]));
	return ret == VALUE_UNDEFINED ? VALUE_NULL : ret;
}

static value builtin_insert_fn(const value *arguments) {
	if (!is_array(arguments[0]))
		die_with_stacktrace("can only `insert` from arrays, not %s", value_name(arguments[0]));

	if (!is_number(arguments[1]))
		die_with_stacktrace("index needs to be an integer for `insert`, not %s", value_name(arguments[1]));

	if (!insert_at_array(as_array(arguments[0]), as_number(arguments[1]), clone_value(arguments[2]))) {
		die_with_stacktrace("cannot insert to negative indicies larger than `ary`'s length: %lld", as_number(arguments[0]));
	}

	return clone_value(arguments[0]);
}

static value builtin_typeof_fn(const value *arguments) {
	const char *typename = value_name(arguments[0]);

	return new_string_value(new_string(strdup(typename), strlen(typename)));
}

#define BUILTIN_FN(name_, argc, fn) \
	(builtin_function) { \
		.name = name_, \
		.required_argument_count = argc, \
		.function_pointer = fn \
	}

builtin_function builtin_functions[] = {
	BUILTIN_FN("to_num", 1, builtin_to_num_fn),
	BUILTIN_FN("prompt", 0, builtin_prompt_fn),
	BUILTIN_FN("print", 1, builtin_print_fn),
	BUILTIN_FN("println", 1, builtin_println_fn),
	BUILTIN_FN("random", 0, builtin_random_fn),
	BUILTIN_FN("length", 1, builtin_length_fn),
	BUILTIN_FN("exit", 1, builtin_exit_fn),
	BUILTIN_FN("dump", 1, builtin_dump_fn),
	BUILTIN_FN("delete", 2, builtin_delete_fn),
	BUILTIN_FN("insert", 3, builtin_insert_fn),
	BUILTIN_FN("typeof", 1, builtin_typeof_fn),
};

