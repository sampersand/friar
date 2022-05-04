#include "number.h"
#include <ctype.h>

string *number_to_string(number num) {
	// Assuming 64 bit `long long`s, the largest string possible is
	// `-9223372036854775807` (LLONG_MIN), which is 20 characters long. Including
	// the ending `0`, the buffer should be 21 chars long.
	char buf[21];

	// We use `snprintf` just in case `long long`s aren't 64 bits for some reason
	snprintf(buf, sizeof(buf), "%lld", num);

	return new_string(strdup(buf), strlen(buf));
}

number string_to_number(const string *str) {
	// We can't use `strtoll` as strings aren't null terminated.

	const char *ptr = str->ptr;
	unsigned length_remaining = str->length;

	// Remove leading whitespace.
	while (length_remaining != 0 && isspace(ptr[0])) {
		ptr++;
		length_remaining--;
	}

	if (length_remaining == 0)
		return 0;

	// Check for leading `-` or `+`s.
	bool is_negative = ptr[0] == '-';
	if (ptr[0] == '-' || ptr[0] == '+') {
		ptr++;
		length_remaining--;
	}

	// Build the number.
	number num = 0;
	while (length_remaining != 0 && isdigit(ptr[0])) {
		num = num * 10 + (ptr[0] - '0');

		ptr++;
		length_remaining--;
	}

	return is_negative ? -num : num;
}
