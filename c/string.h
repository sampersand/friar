#pragma once

#include <string.h>
#include <assert.h>
#include "shared.h"

typedef struct {
	unsigned refcount, length;
	char *ptr;
} string;

string *new_string2(char *ptr, unsigned length);

static inline string *new_string1(char *ptr) {
	return new_string2(ptr, strlen(ptr));
}

static inline string *alloc_string(unsigned capacity) {
	char *c = xmalloc(capacity + 1);
	c[0] = '\0';
	return new_string2(c, 0);
}

void dealloc_string(string *str);

static inline void free_string(string *str) {
#ifndef WE_SOLVED_FREE_ISSUES
	return;
#endif
	assert(str->refcount != 0);

	str->refcount--;
	if (str->refcount == 0)
		dealloc_string(str);
}

static inline string *clone_string(string *str) {
	str->refcount++;
	return str;
}

string *index_string(const string *str, int idx);
string *add_strings(string *lhs, string *rhs);
int compare_strings(const string *lhs, const string *rhs);