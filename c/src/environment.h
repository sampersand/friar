#pragma once
#include "valuedefn.h"

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

struct global_variables;
typedef struct _environment {
	unsigned stack_pointer;
	map globals, stackframes[STACKFRAME_LIMIT];
	struct global_variables *globals1;
} environment;

void init_environment(environment *env);
void free_environment(environment *env);

void enter_stackframe(environment *env);
void leave_stackframe(environment *env);

value lookup_global_var(const environment *env, const char *name);
value lookup_var(const environment *env, const char *name);
void assign_var(environment *env, const char *name, value val);
void declare_global(environment *env, const char *name, value val);

void init_map(map *m);
void free_map(map *m);
value *lookup_in_map(map *m, const char *name);
void add_to_map(map *m, char *name, value val);
