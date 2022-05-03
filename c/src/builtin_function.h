#pragma once

#include "valuedefn.h"

typedef struct {
	VALUE_ALIGNMENT char *name;
	unsigned required_argument_count;
	value (*function_pointer)(const value *arguments);
} builtin_function;

value call_builtin_function(
	builtin_function *builtin_func,
	unsigned number_of_arguments,
	const value *arguments
);

#define NUMBER_OF_BUILTIN_FUNCTIONS 11
extern builtin_function builtin_functions[NUMBER_OF_BUILTIN_FUNCTIONS];
