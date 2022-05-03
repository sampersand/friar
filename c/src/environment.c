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

void enter_stackframe(environment *env, const source_code_location *location) {
	if (env->stack_pointer == STACKFRAME_LIMIT)
		die("stack level too deep (%d levels deep)", STACKFRAME_LIMIT);

	env->stackframes[env->stack_pointer] = location;
	env->stack_pointer++;
}

void leave_stackframe(environment *env) {
	env->stack_pointer--;
}

void dump_stacktrace(FILE *out, const environment *env) {
	for (unsigned i = 0; i < env->stack_pointer; i++) {
		const source_code_location *location = env->stackframes[i];

		fprintf(out, "%d: ", i);

		if (location == NULL) {
			fputs("<unknown>", out);
		} else {
			fprintf(out, "%s:%d", location->filename, location->lineno);
			if (location->function_name != NULL)
				fprintf(out, " in '%s'", location->function_name);
		}

		fputc('\n', out);
	}
}
