#include "env.h"
#include "ast.h"
#include "shared.h"
#include <stdbool.h>
#include <string.h>

void run_declaration(const ast_declaration *d, env *e) {
	if (d->kind == AST_GLOBAL) {
		declare_global(e, d->name, VNULL);
		return;
	}

	declare_global(e, d->name, new_function(d->name, d->argc, d->args, d->block));
}


value run_expression(ast_expression *expr, env *e);
value run_primary(ast_primary *prim, env *e){
	value v1, v2;

	switch (prim->kind) {
	case AST_PAREN:
		return run_expression(prim->expr, e);

	case AST_INDEX:
		v1 = run_primary(prim->prim, e);
		v2 = run_expression(prim->expr, e);
		return index_into(v1, v2);

	// this is bad. 
	case AST_FNCALL: {
		int kind = 0;
		if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "print")) kind = 1;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "push")) kind = 2;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "pop")) kind = 3;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "length")) kind = 4;
		else v1 = run_primary(prim->prim, e);

		value args[prim->amnt];
		for (int i = 0; i < prim->amnt; ++i)
			args[i] = run_expression(prim->args[i], e);

		switch (kind){
		case 0: return call_value(v1, prim->amnt, args, e);
		case 1: printf("%s\n", value2str(args[0])); return VNULL;
		case 2: case 3:
			die("todo(fncall)");
		case 4:
			switch (classify(v1)) {
			case V_STR:
				return num2value(strlen(value2str(args[0])));
			case V_ARY:
				return num2value(value2ary(args[0])->len);
			default:
				die("can only get lengths of arrays and strings");
			}
		}
	}

	case AST_NEG:
		v1 = run_primary(prim->prim, e);
		if (!is_number(v1))
			die("can only negate numbers, not %llx", v1);
		return num2value(-value2num(v1));

	case AST_NOT:
		v1 = run_primary(prim->prim, e);
		if (v1 != VTRUE && v1 != VFALSE && v1 != VNULL)
			die("can only logically negate booleans, not %llx", v1);
		return v1 != VTRUE;

	case AST_ARY:;
		array *a = malloc(sizeof(array));
		a->eles = malloc((a->cap = a->len = prim->amnt) * sizeof(value));
		for (int i = 0; i < a->len; ++i)
			a->eles[i] = run_expression(prim->args[i], e);
		return ary2value(a);
	case AST_VAR:
		if ((v1 = lookup_var(e, prim->ident)) == VUNDEF)
			die("undefined variable '%s' accessed", prim->ident);

		return v1;
	case AST_LITERAL:
		return prim->value;
	}
}

value run_expression(ast_expression *expr, env *e){
	value v, v2, v3;
	switch (expr->kind) {
	case AST_ASSIGN:
		assign_var(e, expr->name, v = run_expression(expr->rhs, e));
		return v;

	case AST_IDX_ASSIGN:
		v = run_primary(expr->prim, e);
		v2 = run_expression(expr->index, e);
		v3 = run_expression(expr->rhs, e);
		index_assign(v, v2, v3);
		return v3;

	case AST_PRIM:
		return run_primary(expr->prim, e);

	case AST_BINOP:
		v = run_primary(expr->prim, e);
		v2 = run_expression(expr->rhs, e);
		switch (expr->binop) {
		case TK_ADD:
			if (classify(v) == V_INT) {
				if (classify(v2) != V_INT) die("can only add ints to ints");
				return num2value(value2num(v) + value2num(v2));
			}

			if (classify(v) == V_ARY) {
				if (classify(v2) != V_ARY) die("can only add arys to arys");
				array *ret = malloc(sizeof(array)), *a = value2ary(v), *b = value2ary(v2);
				ret->eles = malloc((ret->len = ret->cap = a->len+b->len) * sizeof(value));
				memcpy(ret->eles, a->eles, a->len*sizeof(value));
				memcpy(ret->eles + a->len, b->eles, b->len*sizeof(value));
				return ary2value(ret);
			}

			if (classify(v) == V_STR) {
				int len = strlen(value2str(v));
				char *c;
				switch (classify(v2)) {
				case V_NULL:
					strcat(memcpy(c = malloc(len + 5), value2str(v), len + 1), "null");
					break;
				case V_BOOL:
					if (v2 == VTRUE)
						strcat(memcpy(c = malloc(len + 5), value2str(v), len + 1), "true");
					else
						strcat(memcpy(c = malloc(len + 6), value2str(v), len + 1), "false");
					break;
				case V_INT:
					memcpy(c = malloc(47 + len), value2str(v), len + 1);
					sprintf(c + len, "%lld", value2num(v2));
					break;
				case V_STR:
					strcat(memcpy(c = malloc(len + strlen(value2str(v2))), value2str(v), len+1), value2str(v2));
					break;
				default:
					die("todo, convert other types to strings, not %d", classify(v2));
				}
				return str2value(c);
			}
		case TK_SUB:
			if (classify(v) != V_INT || classify(v2) != V_INT) die("can only subtract ints from ints");
			return num2value(value2num(v) - value2num(v2));
		case TK_MUL:
			if (classify(v) != V_INT || classify(v2) != V_INT) die("can only multiply ints with ints");
			return num2value(value2num(v) * value2num(v2));
		case TK_DIV:
			if (classify(v) != V_INT || classify(v2) != V_INT) die("can only divide ints from ints");
			return num2value(value2num(v) / value2num(v2));
		case TK_MOD:
			if (classify(v) != V_INT || classify(v2) != V_INT) die("can only modulo ints from ints");
			return num2value(value2num(v) % value2num(v2));

		case TK_LTH:
			if (classify(v) != classify(v2)) die("can only compare like types");
			if (classify(v) == V_INT) return value2num(v) < value2num(v2) ? VTRUE : VFALSE;
			if (classify(v) == V_STR) return strcmp(value2str(v), value2str(v2)) < 0 ? VTRUE : VFALSE;
			die("can only compare ints and strings (and maybe arrays later)");
		case TK_GTH:
			if (classify(v) != classify(v2)) die("can only compare like types");
			if (classify(v) == V_INT) return value2num(v) > value2num(v2) ? VTRUE : VFALSE;
			if (classify(v) == V_STR) return strcmp(value2str(v), value2str(v2)) > 0 ? VTRUE : VFALSE;
			die("can only compare ints and strings (and maybe arrays later)");
		case TK_LEQ:
			if (classify(v) != classify(v2)) die("can only compare like types");
			if (classify(v) == V_INT) return value2num(v) <= value2num(v2) ? VTRUE : VFALSE;
			if (classify(v) == V_STR) return strcmp(value2str(v), value2str(v2)) <= 0 ? VTRUE : VFALSE;
			die("can only compare ints and strings (and maybe arrays later)");
		case TK_GEQ:
			if (classify(v) != classify(v2)) die("can only compare like types");
			if (classify(v) == V_INT) return value2num(v) >= value2num(v2) ? VTRUE : VFALSE;
			if (classify(v) == V_STR) return strcmp(value2str(v), value2str(v2)) >= 0 ? VTRUE : VFALSE;
			die("can only compare ints and strings (and maybe arrays later)");

		case TK_EQL:
		case TK_NEQ:;
			int eql = v == v2;
			if (classify(v) != classify(v2)) eql = 0;
			else if(classify(v) == V_STR) eql = v == v2 || !strcmp(value2str(v), value2str(v2));
			else if (classify(v) == V_ARY) die("todo, compare arrays");

			if (expr->binop == TK_EQL) eql = !v;
			return eql ? VFALSE : VTRUE;

		default:
			die("unknown operator %d encountered", expr->binop);
		}
		return 0;
	}
}


#define NOTHING 0
#define RETURN_REQUESTED 1
#define BREAK_REQUESTED 2
#define CONTINUE_REQUESTED 3

int run_block(ast_block *block, value *ret, env *e) {
	int retkind;

	for (int i = 0; i < block->amnt; ++i) {
		ast_statement *s = block->stmts[i];

		switch (s->kind) {
		case AST_RETURN:
			*ret = s->expr ? run_expression(s->expr, e) : VNULL;
			return RETURN_REQUESTED;

		case AST_IF:
			if (value2bool(run_expression(s->expr, e)) ? 
				(retkind = run_block(s->body, ret, e)) :
				s->else_body && (retkind=run_block(s->else_body, ret, e)))
				return retkind;
			break;

		case AST_WHILE:
			while (value2bool(run_expression(s->expr, e))) 
				if ((retkind = run_block(s->body, ret, e)) == BREAK_REQUESTED) break;
				else if (retkind == RETURN_REQUESTED) return RETURN_REQUESTED;
			break;

		case AST_BREAK:
			return BREAK_REQUESTED;

		case AST_CONTINUE:
			return CONTINUE_REQUESTED;

		case AST_EXPR:
			run_expression(s->expr, e);
		}
	}

	return false;
}
