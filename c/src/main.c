#include "token.h"
#include "value.h"
#include "compile.h"
#include "codeblock.h"
#include "environment.h"
#include "globals.h"

int main(int argc, char **argv) {
	if (argc != 3)
		die("usage: %s -e <program>", argv[0]);

	init_environment();
	init_global_variables();

	compile("-e", argv[2]);

	int main_index = lookup_global_variable("main");
	if (main_index == -1)
		die("you must define a `main` function");

	value ret = call_value(fetch_global_variable(main_index), 0, NULL);

	free_environment();
	free_global_variables();
	
	if (is_number(ret))
		return as_number(ret);
}
