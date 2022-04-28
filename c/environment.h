#pragma once
#include "value.h"

#ifndef STACKFRAME_LIMIT
#	define STACKFRAME_LIMIT 10000
#endif

typedef struct {
	char *name;
	value val;
} map_entry;

typedef struct {
	unsigned cap, len;
	map_entry *entries;
} map;

typedef struct _environment {
	unsigned stack_pointer;
	map globals, stackframes[STACKFRAME_LIMIT];
} environment;

void init_environment(environment *env);
void free_environment(environment *env);

void enter_stackframe(environment *env);
void leave_stackframe(environment *env);

value lookup_global_var(const environment *env, const char *name);
value lookup_var(const environment *env, const char *name);
void assign_var(environment *env, const char *name, value val);
void declare_global(environment *env, const char *name, value val);
