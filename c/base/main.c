#include "base/token.h"
#include "base/value.h"
#include "bytecode/compile.h"
#include "bytecode/codeblock.h"

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
		dump_ast_declaration(stdout, d);
		compile_declaration(&comp, d);
	}


	// 	exit(0);

	value *v;
	if ((v = lookup_in_map(&comp.globals, "main")) == NULL)
		die("you must define a `main` function");

	value ret = call_value(*v, 0, 0, &env);
	if (is_number(ret))
		return as_number(ret);
	// free_environment(&env);
}
