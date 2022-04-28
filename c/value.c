#include <stdlib.h>
#include <string.h>
#include "shared.h"
#include "value.h"
#include "ast.h"
#include "env.h"

void dump_value(FILE *out, value v) {
	fprintf(out, "<value:%08llx>", v);
}

typedef struct {
	char *name, **argv;
	int argc;
	ast_block *block;
} function;

value new_function(char *name, int argc, char **argv, ast_block *block) {	
	function *f = malloc(sizeof(function));
	f->name = name;
	f->argc = argc;
	f->argv = argv;
	f->block = block;

	return (value) f | 1;
}

value run_block(ast_block *, value *, env *);
value call_value(value v, int argc, value *argv, env *e) {
	if (classify(v) != V_FUNC)
		die("cannot call invalid value: %llx", v);

	function *f = (function *) (v & ~1);
	if (f->argc != argc)
		die("argument mismatch for %s: expected %d, got %d", f->name, f->argc, argc);

	++e->sp;
	for (int i = 0; i < argc; ++i)
		assign_var(e, f->argv[i], argv[i]);
	value ret;
	run_block(f->block, &ret, e);
	--e->sp;
	return ret;
}

void index_assign(value ary, value idx, value val) {
	if (classify(ary) != V_ARY) die("can only index assign into arrays");
	if (classify(idx) != V_INT) die("you must index with numbers");

	long long i = value2num(idx);
	array *a = value2ary(ary);

	if (i < 0) die("negative indexing isnt supported rn");
	if (a->len <= i) {
		if (a->cap <= i)
			a->eles = realloc(a->eles, (a->cap = i) * sizeof(value));
		while (a->len <= i)
			a->eles[a->len++] = VNULL;
	}

	a->eles[i] = val;
}

value index_into(value ary, value idx) {
	if (classify(idx) != V_INT) die("you must index with numbers");

	long long i = value2num(idx);
	if (i < 0) die("negative indexing isnt supported rn");

	switch (classify(ary)) {
	case V_STR:;
		char *s = value2str(ary);
		if (strlen(s) <= i) return VNULL;
		char *c = malloc(2);
		c[0] = s[i];
		c[1] = '\0';
		return str2value(c);

	case V_ARY:;
		array *a = value2ary(ary);
		return a->len <= i ? VNULL : a->eles[i];
	default:
		die("can only index into arrays or strs");
	}
}
