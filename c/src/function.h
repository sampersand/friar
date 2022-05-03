#pragma once

#include <assert.h>
#include "ast.h"
#include "environment.h"
#include "valuedefn.h"
#include "codeblock.h"
#include <stdalign.h>

value run_codeblock(const codeblock *block, unsigned argc, const value *argv, environment *env);
void free_codeblock(codeblock *block);

typedef struct {
	VALUE_ALIGNMENT codeblock *body;
	char *name, **argv;
	unsigned argc, refcount;
} function;

function *new_function(char *name, unsigned argc, char **argv, codeblock *body);

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
