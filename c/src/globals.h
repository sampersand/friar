#pragma once
#include "valuedefn.h"

void init_global_variables(void);
void free_global_variables(void);

// returns the index of the global variable, creating it with a default of VNULL if it doesnt exist.
unsigned declare_global_variable(char *name);

int lookup_global_variable(const char *name);
void assign_global_variable(unsigned index, value val);
value fetch_global_variable(unsigned index);
