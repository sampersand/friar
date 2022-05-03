#include "token.h"
#include "value.h"
#include "compile.h"
#include "codeblock.h"

void run_declaration(ast_declaration*, environment*);
// value run_codeblock(const codeblock *block, environment *env);

environment env;
int main(int argc, char **argv) {
	(void) argc;
	tokenizer tzr = new_tokenizer(argv[1]);

	compiler comp;
	init_compiler(&comp);
	init_environment(&env);
	ast_declaration *d;
	while ((d = next_declaration(&tzr))) {
#ifdef ENABLE_LOGGING
		dump_ast_declaration(stdout, d);
#endif
		compile_declaration(&comp, d);
	}

	env.globals1 = comp.globals;

	// 	exit(0);

	int main_index = lookup_global_variable(comp.globals, "main");
	if (main_index == -1)
		die("you must define a `main` function");

	value ret = call_value(fetch_global_variable(comp.globals, main_index), 0, NULL, &env);
	if (is_number(ret))
		return as_number(ret);
	// free_environment(&env);
}
