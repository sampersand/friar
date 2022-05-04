#pragma once

#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "shared.h"
#include "valuedefn.h"

// Note that strings in friar are not nul terminated, and as such aren't compatible with any of the
// builtin `strxxx` family of functions (eg `strdup`).
typedef struct {
	VALUE_ALIGNMENT char *ptr;
	unsigned refcount, length;
} string;

string *new_string(char *ptr, unsigned length);

static inline string *allocate_string(unsigned capacity) {
	return new_string(xmalloc(capacity), 0);
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
bool equate_strings(const string *lhs, const string *rhs);
string *replicate_string(string *str, unsigned amnt);
