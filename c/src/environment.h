#pragma once
#include "valuedefn.h"
#include "globals.h"

#ifndef STACKFRAME_LIMIT
#	define STACKFRAME_LIMIT 10000
#endif

typedef struct {
	char *filename;
	unsigned lineno;
} source_code_location;

typedef struct {
	unsigned stack_pointer;
	source_code_location stackframes[STACKFRAME_LIMIT];
	global_variables *globals1;
} environment;

void init_environment(environment *env);
void free_environment(environment *env);

void enter_stackframe(environment *env);
void leave_stackframe(environment *env);
