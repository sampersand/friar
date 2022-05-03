#pragma once

#include <stdio.h>
#include "globals.h"

#define edie(env, ...) (fprintf(stderr, __VA_ARGS__), fputs("\nstacktrace:\n", stderr), \
	dump_stacktrace(stderr),exit(1))

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

extern environment global_environment;

void enter_stackframe(const source_code_location *location);
void leave_stackframe(void);
void dump_stacktrace(FILE *out);
