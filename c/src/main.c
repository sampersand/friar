#include "token.h"
#include "value.h"
#include "compile.h"
#include "codeblock.h"
#include "environment.h"

int main(int argc, char **argv) {
	(void) argc;
	tokenizer tzr = new_tokenizer(argv[2], "-e");

	compiler comp;
	init_compiler(&comp);

	ast_declaration *d;
	while ((d = next_declaration(&tzr))) {
#ifdef ENABLE_LOGGING
		dump_ast_declaration(stdout, d);
#endif
		compile_declaration(&comp, d);
	}

	global_environment.globals = comp.globals;

	int main_index = lookup_global_variable(comp.globals, "main");
	if (main_index == -1)
		die("you must define a `main` function");

	value ret = call_value(fetch_global_variable(comp.globals, main_index), 0, NULL);
	if (is_number(ret))
		return as_number(ret);
}
