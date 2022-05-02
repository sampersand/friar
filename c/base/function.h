#pragma once

#include <assert.h>
#include "base/ast.h"
#include "base/environment.h"

#ifdef AST_WALKER
typedef ast_block function_body;
#else
typedef struct codeblock function_body;
value run_codeblock(const function_body *block, environment *env);
void free_codeblock(function_body *block);
#endif

typedef struct {
	char *name, **argv;
	unsigned argc, refcount;
	function_body *body;
} function;

function *new_function(char *name, unsigned argc, char **argv, function_body *body);

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

value call_function(const function *func, unsigned argc, value *argv, environment *env);
