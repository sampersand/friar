#pragma once

#include <stdio.h>
#include "globals.h"

#define edie(env, ...) (fprintf(stderr, __VA_ARGS__), fputs("\nstacktrace:\n", stderr), \
	dump_stacktrace(stderr, env),exit(1))

#ifndef STACKFRAME_LIMIT
# define STACKFRAME_LIMIT 1000
#endif

typedef struct {
	const char *filename, *function_name;
	unsigned lineno;
} source_code_location;

typedef struct {
	unsigned stack_pointer;
	const source_code_location *stackframes[STACKFRAME_LIMIT];
	global_variables *globals;
} environment;

void init_environment(environment *env);
void free_environment(environment *env);

void enter_stackframe(environment *env, const source_code_location *location);
void leave_stackframe(environment *env);
void dump_stacktrace(FILE *out, const environment *env);
