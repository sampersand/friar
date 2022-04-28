#include "env.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

value lookup_var(env *e, const char *s) {
	if (e->sp) {
		map *locals = &e->stackframes[e->sp];

		for (int i = 0; i < locals->len; ++i)
			if (!strcmp(locals->entries[i].name, s))
				return locals->entries[i].v;
	}

	for (int i = 0; i < e->globals.len; ++i)
		if (!strcmp(e->globals.entries[i].name, s))
			return e->globals.entries[i].v;

	return VUNDEF;
}

void assign_var(env *e, const char *s, value v) {
	for (int i = 0; i < e->globals.len; ++i)
		if (!strcmp(e->globals.entries[i].name, s)) {
			e->globals.entries[i].v = v;
			return;
		}

	map *locals = &e->stackframes[e->sp];

	for (int i = 0; i < locals->len; ++i)
		if (!strcmp(locals->entries[i].name, s)) {
			locals->entries[i].v = v;
			return;
		}

	if (locals->len == locals->cap)
		locals->entries = realloc(locals->entries, (locals->cap=locals->cap*2+1) * sizeof(map_entry));

	locals->entries[locals->len++] = (map_entry) { .name = s, .v = v };
}

void declare_global(env *e, const char *s, value v) {
	for (int i = 0; i < e->globals.len; ++i)
		if (!strcmp(e->globals.entries[i].name, s))
			return;

	if (e->globals.len == e->globals.cap) 
		e->globals.entries = realloc(e->globals.entries, (e->globals.cap = e->globals.cap*2+1) * sizeof(map_entry));

	e->globals.entries[e->globals.len++] = (map_entry) { .name = s, .v = v };
}
