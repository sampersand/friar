#pragma once
#include "valuedefn.h"

typedef struct {
	char *name;
	value val;
} global_variable_entry;

typedef struct global_variables {
	unsigned length, capacity;
	global_variable_entry *entries;
} global_variables;

global_variables *new_global_variables(void);
void free_global_variables(global_variables *globals);

// returns the index of the global variable, creating it with a default of NULL if it doesnt exist.
unsigned declare_global_variable(global_variables *globals, char *name);
int lookup_global_variable(const global_variables *globals, const char *name);
void assign_global_variable(global_variables *globals, unsigned index, value val);
value fetch_global_variable(const global_variables *globals, unsigned index);
