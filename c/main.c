#include "token.h"
#include "ast.h"
#include "environment.h"
#include "shared.h"

void run_declaration(ast_declaration*, environment*);

environment env;
int main(int argc, char **argv) {
	tokenizer tzr = new_tokenizer(argv[1]);

	init_environment(&env);
	ast_declaration *d;
	while ((d = next_declaration(&tzr)))
		run_declaration(d, &env);

	value v;
	if ((v = lookup_global_var(&env, "main")) == VUNDEF)
		die("you must define a `main` function");
	call_value(v, 0, 0, &env);
	free_environment(&env);
}
