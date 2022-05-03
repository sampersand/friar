#include "environment.h"
#include "value.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void init_environment(environment *env) {
	env->stack_pointer = 0;
}

void free_environment(environment *env) {
	assert(env->stack_pointer == 0);
}

void enter_stackframe(environment *env) {
	env->stack_pointer++;
}

void leave_stackframe(environment *env) {
	env->stack_pointer--;
}

