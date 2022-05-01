#include "base/environment.h"
#include "base/value.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static void init_map(map *m) {
	m->len = 0;
	m->cap = 0;
	m->entries = NULL;
}

static void free_map(map *m) {
	for (unsigned i = 0; i < m->len; i++) {
		free(m->entries[i].name);
		free_value(m->entries[i].val);
	}

	free(m->entries);
}

static value *lookup_in_map(map *m, const char *name) {
	for (unsigned i = 0; i < m->len; i++) {
		if (!strcmp(m->entries[i].name, name))
			return &m->entries[i].val;
	}

	return NULL;
}

static void add_to_map(map *m, char *name, value val) {
	assert(lookup_in_map(m, name) == NULL);

	if (m->len == m->cap) {
		m->cap = m->cap * 2 + 1;
		m->entries = realloc(m->entries, m->cap * sizeof(map_entry));
	}

	m->entries[m->len++] = (map_entry) { .name = name, .val = val };
}

static map *current_stackframe(environment *env) {
	return &env->stackframes[env->stack_pointer];
}

static value *lookup_local_or_global_var(environment *env, const char *name) {
	value *ptr = lookup_in_map(current_stackframe((environment *) env), name);

	if (ptr != NULL)
		return ptr;

	return lookup_in_map((map *) &env->globals, name);
}

void init_environment(environment *env) {
	env->stack_pointer = 0;
	init_map(&env->globals);
}

void free_environment(environment *env) {
	assert(env->stack_pointer == 0);
	free_map(&env->globals);
}

void enter_stackframe(environment *env) {
	env->stack_pointer++;
	init_map(current_stackframe(env));
}

void leave_stackframe(environment *env) {
	free_map(current_stackframe(env));
	env->stack_pointer--;
}


value lookup_global_var(environment const *env, const char *name) {
	value *ptr = lookup_in_map((map *) &env->globals, name);
	return ptr == NULL ? VUNDEF : *ptr;
}

value lookup_var(const environment *env, const char *name) {
	value *ptr = lookup_local_or_global_var((environment *) env, name);
	return ptr == NULL ? VUNDEF : *ptr;
}

void assign_var(environment *env, const char *name, value val) {
	value *ptr = lookup_local_or_global_var(env, name);

	if (ptr != NULL) {
		free_value(*ptr);
		*ptr = val;
		return;
	}

	add_to_map(current_stackframe(env), strdup(name), val);
}

void declare_global(environment *env, const char *name, value val) {
	value *ptr = lookup_in_map(&env->globals, name);

	if (ptr != NULL) {
		free_value(*ptr);
		*ptr = val;
		return;
	}

	add_to_map(&env->globals, strdup(name), val);
}
