#include "function.h"
#include "shared.h"
#include <assert.h>
#include <string.h>


function *new_function(
	char *function_name,
	unsigned number_of_arguments,
	char **argument_names,
	codeblock *body,
	char *source_filename,
	unsigned source_lineno
) {
	assert(strlen(function_name) != 0);

	function *func = xmalloc(sizeof(function));

	func->function_name = function_name;
	func->number_of_arguments = number_of_arguments;
	func->argument_names = argument_names;
	func->refcount = 1;
	func->body = body;
	func->source_filename = source_filename;
	func->source_lineno = source_lineno;

	return func;
}

void deallocate_function(function *func) {
	assert(func->refcount == 0);

	for (unsigned i = 0; i < func->number_of_arguments; i++)
		free(func->argument_names[i]);

	free(func->argument_names);
	free(func->function_name);
	free(func->source_filename);
	free_codeblock(func->body);
	free(func);
}

value call_function(const function *func, unsigned number_of_arguments, value *argv) {
	if (func->number_of_arguments != number_of_arguments) {
		edie("argument mismatch for %s: expected %d, got %d",
			func->function_name, func->number_of_arguments, number_of_arguments);
	}


	source_code_location location = {
		.filename = func->source_filename,
		.function_name = func->function_name,
		.lineno = func->source_lineno
	};

	enter_stackframe(&location);
	value ret = run_codeblock(func->body, number_of_arguments, argv);
	leave_stackframe();

	return ret;
}
