#include "environment.h"
#include "value.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

environment global_environment = {
	.stack_pointer = 0
};

void enter_stackframe(const source_code_location *location) {
	if (global_environment.stack_pointer == STACKFRAME_LIMIT)
		die("stack level too deep (%d levels deep)", STACKFRAME_LIMIT);

	global_environment.stackframes[global_environment.stack_pointer] = location;
	global_environment.stack_pointer++;
}

void leave_stackframe(void) {
	assert(global_environment.stack_pointer != 0);
	global_environment.stack_pointer--;
}

void dump_stacktrace(FILE *out) {
	for (unsigned i = 0; i < global_environment.stack_pointer; i++) {
		const source_code_location *location = global_environment.stackframes[i];

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
