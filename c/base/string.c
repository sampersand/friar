#include "base/string.h"
#include "base/shared.h"
#include <assert.h>

string *new_string2(char *ptr, unsigned length) {
	assert(strlen(ptr) == length);

	string *str = xmalloc(sizeof(string));
	
	str->refcount = 0;
	str->length = length;
	str->ptr = ptr;

	return str;
}

void deallocate_string(string *str) {
	assert(str->refcount == 0);

	free(str->ptr);
	free(str);
}

string *index_string(const string *str, int idx) {
	if (idx < 0)
		die("negative indexing of strings isn't currently supported");

	if (str->length <= (unsigned) idx)
		return NULL;

	string *ret = allocate_string(1);
	ret->ptr[0] = str->ptr[idx];
	ret->ptr[1] = '\0';

	return ret;
}

string *add_strings(string *lhs, string *rhs) {
	if (lhs->length == 0) return clone_string(rhs);
	if (rhs->length == 0) return clone_string(lhs);

	string *str = allocate_string(lhs->length + rhs->length);
	memcpy(str->ptr, lhs->ptr, lhs->length);
	memcpy(str->ptr + lhs->length, rhs->ptr, rhs->length + 1); // `+1` for `\0`.

	return str;
}

int compare_strings(const string *lhs, const string *rhs) {
	int cmp = strcmp(lhs->ptr, rhs->ptr);

	return cmp < 0 ? -1 : cmp == 0 ? 0 : 1;
}

string *replicate_string(string *str, unsigned amnt) {
	if (amnt == 1)
		return clone_string(str);

	string *ret = allocate_string(str->length * amnt);

	for (unsigned i = 0; i < amnt; i++)
		memcpy(ret->ptr + i*str->length, str->ptr, str->length);

	ret->length = str->length * amnt;
	ret->ptr[ret->length] = '\0';

	return ret;
}
