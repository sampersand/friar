#pragma once

#include <string.h>
#include <assert.h>

typedef struct {
	unsigned refcount, length;
	char *ptr;
} string;

string *new_string2(char *ptr, unsigned length);

static inline string *new_string1(char *ptr) {
	return new_string2(ptr, strlen(ptr));
}

void dealloc_string(string *str);

static inline void free_string(string *str) {
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
