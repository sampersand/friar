#include "base/token.h"
#include "base/value.h"

void run_declaration(ast_declaration*, environment*);

environment env;
int main(int argc, char **argv) {
	(void) argc;
	tokenizer tzr = new_tokenizer(argv[1]);

	init_environment(&env);
	ast_declaration *d;
	while ((d = next_declaration(&tzr))) {
		dump_ast_declaration(stdout, d);
		// run_declaration(d, &env);
	}
		exit(0);

	value v;
	if ((v = lookup_global_var(&env, "main")) == VUNDEF)
		die("you must define a `main` function");

	call_value(v, 0, 0, &env);
	free_environment(&env);
}
