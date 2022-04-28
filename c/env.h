#pragma once
#include "value.h"

#ifndef STACKFRAME_LIMIT
#	define STACKFRAME_LIMIT 10000
#endif

typedef struct {
	const char *name;
	value v;
} map_entry;

typedef struct {
	unsigned cap, len;
	map_entry *entries;
} map;

typedef struct _env {
	unsigned stack_pointer;
	map globals, stackframes[STACKFRAME_LIMIT];
} env;

value lookup_var(env *, const char *);
void assign_var(env *, const char *, value);
void declare_global(env *, const char *, value);
