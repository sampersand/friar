#pragma once
#include "string.h"

typedef long long number;

static inline int compare_numbers(number lhs, number rhs) {
	return lhs == rhs ? 0 : lhs < rhs ? -1 : 1;
}

string *number_to_string(number num);
