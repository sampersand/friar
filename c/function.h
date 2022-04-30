#pragma once

#include <assert.h>
#include "ast.h"
#include "environment.h"

typedef struct {
	char *name, **argv;
	unsigned argc, refcount;
	ast_block *body;
} function;

function *new_function(char *name, unsigned argc, char **argv, ast_block *body);

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
