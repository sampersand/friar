#include "globals.h"
#include "base/shared.h"
#include "base/value.h"

global_variables *new_global_variables(void) {
	global_variables *globals = xmalloc(sizeof(global_variables));

	globals->length = 0;
	globals->capacity = 8;
	globals->entries = xmalloc(globals->capacity * sizeof(global_variable_entry));

	return globals;
}

void free_global_variables(global_variables *globals) {
	for (unsigned i = 0; i < globals->length; i++) {
		free(globals->entries[i].name);
		free_value(globals->entries[i].val);
	}

	free(globals->entries);
	free(globals);
}


int lookup_global_variable(const global_variables *globals, const char *name) {
	for (unsigned i = 0; i < globals->length; i++) {
		if (!strcmp(name, globals->entries[i].name))
			return i;
	}

	return -1;
}

unsigned declare_global_variable(global_variables *globals, char *name) {
	int previous_index = lookup_global_variable(globals, name);
	if (previous_index != -1)
		return previous_index;

	if (globals->length == globals->capacity) {
		globals->capacity *= 2;
		globals->entries = xrealloc(globals->entries, globals->capacity * sizeof(global_variable_entry));
	}

	unsigned index = globals->length;
	globals->entries[index].name = name;
	globals->entries[index].val = VNULL;
	globals->length++;
	return index;
}

void assign_global_variable(global_variables *globals, unsigned index, value val) {
	assert(index < globals->length);

	free_value(globals->entries[index].val);
	globals->entries[index].val = val;
}

value fetch_global_variable(const global_variables *globals, unsigned index) {
	assert(index < globals->length);

	return globals->entries[index].val;
}
