#pragma once

#include <assert.h>
#include "ast.h"
#include "valuedefn.h"
#include "codeblock.h"
#include <stdalign.h>

typedef struct {
	VALUE_ALIGNMENT codeblock *body;
	char *function_name;
	unsigned refcount;

	unsigned number_of_arguments;
	char **argument_names;

	unsigned source_lineno;
	char *source_filename;
} function;

function *new_function(
	char *function_name,
	unsigned number_of_arguments,
	char **argument_names,
	codeblock *body,
	char *source_filename,
	unsigned source_lineno
);

void deallocate_function(function *func);

static inline void free_function(function *func) {
#ifndef WE_SOLVED_FREE_ISSUES
	return;
#endif

	assert(func->refcount != 0);

	func->refcount--;
	if (func->refcount == 0)
		deallocate_function(func);
}

static inline function *clone_function(function *func) {
	func->refcount++;
	return func;
}

value call_function(const function *func, unsigned argc, value *argv);
