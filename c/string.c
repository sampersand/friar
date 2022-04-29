#include "string.h"
#include "shared.h"
#include <assert.h>

string *new_string2(char *ptr, unsigned length) {
	assert(strlen(ptr) == length);

	string *str = xmalloc(sizeof(string));
	
	str->refcount = 0;
	str->length = length;
	str->ptr = ptr;

	return str;
}

void dealloc_string(string *str) {
	assert(str->refcount == 0);

	free(str->ptr);
	free(str);
}

string *index_string(const string *str, int idx) {
	if (idx < 0)
		die("negative indexing of strings isn't currently supported");

	if (str->length <= idx)
		return NULL;

	char *ret = xmalloc(2);
	ret[0] = str->ptr[idx];
	ret[1] = '\0';

	return new_string2(ret, 1);
}

