#include <stdbool.h>
#include <string.h>

#include "environment.h"
#include "ast.h"
#include "shared.h"
#include "value.h"

void run_declaration(const ast_declaration *d, environment *e) {
	if (d->kind == AST_GLOBAL) {
		declare_global(e, d->name, VNULL);
		return;
	}

	declare_global(e, d->name, new_function_value(new_function(d->name, d->argc, d->args, d->block)));
}


value run_expression(ast_expression *expr, environment *e);
value run_primary(ast_primary *prim, environment *e){
	value v1, v2;

	switch (prim->kind) {
	case AST_PAREN:
		return run_expression(prim->expr, e);

	case AST_INDEX:
		v1 = run_primary(prim->prim, e);
		v2 = run_expression(prim->expr, e);
		return index_value(v1, v2);

	// this is bad. 
	case AST_FNCALL: {
		int kind = 0;
		if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "print")) kind = 1;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "push")) kind = 2;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "pop")) kind = 3;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "length")) kind = 4;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "exit")) kind = 5;
		else if (prim->prim->kind == AST_VAR && !strcmp(prim->prim->ident, "kindof")) kind = 6;
		else v1 = run_primary(prim->prim, e);

		value args[prim->amnt+1]; // `+1` is bc you cant have zero-sized vla
		for (int i = 0; i < prim->amnt; i++)
			args[i] = run_expression(prim->args[i], e);

		switch (kind){
		case 0: return call_value(v1, prim->amnt, args, e);
		case 1: printf("%s\n", as_string(args[0])->ptr); return VNULL;
		case 2: case 3:
			die("todo(fncall)");

		case 5:
			exit(as_number(args[0]));

		case 6:
			return new_string_value(new_string1(strdup(value_name(args[0]))));

		case 4:
			switch (classify(args[0])) {
			case VK_STRING:
				return new_number_value(as_string(args[0])->length);
			case VK_ARRAY:
				return new_number_value(as_array(args[0])->length);
			default:
				die("can only get lengths of arrays and strings");
			}
		}
	}

	case AST_NEG:
		v1 = run_primary(prim->prim, e);
		if (!is_number(v1))
			die("can only negate numbers, not %llx", v1);
		return new_number_value(-as_number(v1));

	case AST_NOT:
		v1 = run_primary(prim->prim, e);
		if (v1 != VTRUE && v1 != VFALSE && v1 != VNULL)
			die("can only logically negate booleans, not %llx", v1);
		return new_boolean_value(!as_boolean(v1));

	case AST_ARY:;
		array *a = malloc(sizeof(array));
		a->elements = malloc((a->capacity = a->length = prim->amnt) * sizeof(value));
		for (int i = 0; i < a->length; i++)
			a->elements[i] = run_expression(prim->args[i], e);
		return new_array_value(a);

	case AST_VAR:
		if ((v1 = lookup_var(e, prim->ident)) == VUNDEF)
			die("undefined variable '%s' accessed", prim->ident);
		return v1;

	case AST_LITERAL:
		return prim->value;
	}
}

value run_expression(ast_expression *expr, environment *e){
	value v, v2, v3;
	switch (expr->kind) {
	case AST_ASSIGN:
		if (expr->binop == TK_ASSIGN) {
			v = run_expression(expr->rhs, e);
		} else {
			if ((v = lookup_var(e, expr->name)) == VUNDEF)
				die("undefined variable '%s' accessed", expr->name);
			v2 = run_expression(expr->rhs, e);
			switch (expr->binop) {
			case TK_ADD_ASSIGN: v = add_values(v, v2); break;
			case TK_SUBTRACT_ASSIGN: v = subtract_values(v, v2); break;
			case TK_MULTIPLY_ASSIGN: v = multiply_values(v, v2); break;
			case TK_DIVIDE_ASSIGN: v = divide_values(v, v2); break;
			case TK_MODULO_ASSIGN: v = modulo_values(v, v2); break;
			default: die("[bug] unknown binop %d", expr->binop);
			}
		}
		assign_var(e, expr->name, v);
		return v;

	case AST_IDX_ASSIGN:
		if (expr->binop != TK_ASSIGN)
			die("todo: non-tk-assign for arrays");

		v = run_primary(expr->prim, e);
		v2 = run_expression(expr->index, e);
		v3 = run_expression(expr->rhs, e);
		index_assign_value(v, v2, v3);
		return v3;

	case AST_PRIM:
		return run_primary(expr->prim, e);

	case AST_BINOP:
		v = run_primary(expr->prim, e);
		v2 = run_expression(expr->rhs, e);
		switch (expr->binop) {
		case TK_ADD: return add_values(v, v2);
		case TK_SUBTRACT: return subtract_values(v, v2);
		case TK_MULTIPLY: return multiply_values(v, v2);
		case TK_DIVIDE: return divide_values(v, v2);
		case TK_MODULO: return modulo_values(v, v2);
		case TK_LESS_THAN: return new_boolean_value(compare_values(v, v2) < 0);
		case TK_GREATER_THAN: return new_boolean_value(compare_values(v, v2) > 0);
		case TK_LESS_THAN_OR_EQUAL: return new_boolean_value(compare_values(v, v2) <= 0);
		case TK_GREATER_THAN_OR_EQUAL: return new_boolean_value(compare_values(v, v2) >= 0);
		case TK_EQUAL: return new_boolean_value(equate_values(v, v2));
		case TK_NOT_EQUAL: return new_boolean_value(!equate_values(v, v2));
		default: die("[bug] unknown operator %d encountered", expr->binop);
		}
	}
}


#define NOTHING 0
#define RETURN_REQUESTED 1
#define BREAK_REQUESTED 2
#define CONTINUE_REQUESTED 3

int run_block(const ast_block *block, value *ret, environment *e) {
	int retkind;

	for (int i = 0; i < block->amnt; i++) {
		ast_statement *s = block->stmts[i];

		switch (s->kind) {
		case AST_RETURN:
			*ret = s->expr ? run_expression(s->expr, e) : VNULL;
			return RETURN_REQUESTED;

		case AST_IF:
			if (as_boolean(run_expression(s->expr, e)) ? 
				(retkind = run_block(s->body, ret, e)) :
				s->else_body && (retkind=run_block(s->else_body, ret, e)))
				return retkind;
			break;

		case AST_WHILE:
			while (as_boolean(run_expression(s->expr, e))) 
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
