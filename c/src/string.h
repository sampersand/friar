#pragma once

#include <string.h>
#include <assert.h>
#include "shared.h"
#include "valuedefn.h"

typedef struct {
	VALUE_ALIGNMENT char *ptr;
	unsigned refcount, length;
} string;

string *new_string2(char *ptr, unsigned length);

static inline string *new_string1(char *ptr) {
	return new_string2(ptr, strlen(ptr));
}

static inline string *allocate_string(unsigned capacity) {
	char *c = xmalloc(capacity + 1);

#ifndef NDEBUG
	// This is required because `new_string2` has an `assert(strlen)` in it.
	c[0] = '\0';
#endif

	return new_string2(c, 0);
}

void deallocate_string(string *str);

static inline void free_string(string *str) {
#ifndef WE_SOLVED_FREE_ISSUES
	return;
#endif
	assert(str->refcount != 0);

	str->refcount--;
	if (str->refcount == 0)
		deallocate_string(str);
}

static inline string *clone_string(string *str) {
	str->refcount++;
	return str;
}

string *index_string(const string *str, int idx);
string *add_strings(string *lhs, string *rhs);
int compare_strings(const string *lhs, const string *rhs);
string *replicate_string(string *str, unsigned amnt);
