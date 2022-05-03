#pragma once

#include "base/environment.h"
#include "base/valuedefn.h"

typedef struct {
	VALUE_ALIGNMENT char *name;
	unsigned required_argument_count;
	value (*function_pointer)(const value *arguments, environment *env);
} builtin_function;

value call_builtin_function(
	builtin_function *builtin_func,
	unsigned number_of_arguments,
	const value *arguments,
	environment *env
);

#define NUMBER_OF_BUILTIN_FUNCTIONS 8
extern builtin_function builtin_functions[NUMBER_OF_BUILTIN_FUNCTIONS];
