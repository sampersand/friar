#include "token.h"
#include "ast.h"
#include "env.h"
#include "shared.h"

env e;
void run_declaration(ast_declaration*, env*);
int main(int argc, char **argv) {
	// tokenizer tzr = new_tokenizer("function main() { b = 0; }");
	tokenizer tzr = new_tokenizer(argv[1]);
	// tokenizer tzr = new_tokenizer("\
	// 	function foo() { x = 2; return 4; } \n\
	// 	function main1(){ x = 3; print(\"hi\" + (''+foo()) + \"\n\"); print(\"\"+x); } \n\
	// 	function main() { \n\
	// 		ary = [1, 2, 3]; \n\
	// 		a = 0;\n\
	// 		# i = 0; \n\
	// 		# while i < 3 { print('' + ary[i]); i = i+1; } \n\
	// 	}");

	ast_declaration *d;
	while ((d = next_declaration(&tzr)))
		run_declaration(d, &e);

	value v;
	if ((v = lookup_var(&e, "main")) == VUNDEF)
		die("you must define a `main` function");
	call_value(v, 0, 0, &e);
}
