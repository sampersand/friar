#include "function.h"
#include "shared.h"
#include <assert.h>
#include <string.h>

function *new_function(char *name, unsigned argc, char **argv, function_body *body) {
	assert(name == NULL || strlen(name) != 0);

	function *func = xmalloc(sizeof(function));

	func->name = name;
	func->argc = argc;
	func->argv = argv;
	func->refcount = 0;
	func->body = body;

	return func;
}

void deallocate_function(function *func) {
	assert(func->refcount == 0);

	for (unsigned i = 0; i < func->argc; i++)
		free(func->argv[i]);

	free(func->argv);
	free(func->name);
#ifdef AST_WALKER
	free_ast_block(func->body);
#else
	free_codeblock(func->body);
#endif
	free(func);
}

value call_function(const function *func, unsigned argc, value *argv, environment *env) {
	if (func->argc != argc)
		die("argument mismatch for %s: expected %d, got %d", func->name, func->argc, argc);

	enter_stackframe(env);

	value ret;

#ifdef AST_WALKER
	for (unsigned i = 0; i < argc; i++)
		assign_var(env, func->argv[i], argv[i]);

	run_block(func->body, &ret, env);
#else
	ret = run_codeblock(func->body, argc, argv, env);
#endif

	leave_stackframe(env);
	return ret;
}
