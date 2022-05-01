#include "base/number.h"

string *number_to_string(number num) {
	// Assuming 64 bit `long long`s, the largest string possible is
	// `-9223372036854775807` (LLONG_MIN), which is 20 characters long. Including
	// the ending `0`, the buffer should be 21 chars long.
	char buf[21];

	// We use `snprintf` just in case `long long`s aren't 64 bits for some reason
	snprintf(buf, sizeof(buf), "%lld", num);

	return new_string1(strdup(buf));
}
