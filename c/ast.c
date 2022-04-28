#include "ast.h"
#include "token.h"
#include <stdlib.h>
#include <assert.h>

#define UNEXPECTED_TOKEN(tzr, tkn) (fprintf(stderr, \
	"unexpected token at line %d: [%d] ", tzr->lineno, __LINE__), dump_token(stderr, tkn), exit(1))

token peek(tokenizer *tzr) {
	if (!tzr->prev.kind)
		tzr->prev = next_token(tzr);
	return tzr->prev;
}

token advance(tokenizer *tzr) {
	token tkn = peek(tzr);
	tzr->prev.kind = TK_EOF;
	return tkn;
}

void unadvance(tokenizer *tzr, token tkn) {
	assert(!tzr->prev.kind);
	tzr->prev = tkn;
}

token guard(tokenizer *tzr, token_kind k) {
	return peek(tzr).kind == k ? advance(tzr) : (token) { .kind = TK_EOF };
}

token expect(tokenizer *tzr, token_kind k) {
	token tkn = guard(tzr, k);

	if (tkn.kind)
		return tkn;

	UNEXPECTED_TOKEN(tzr, peek(tzr));
}

static ast_expression *parse_expression(tokenizer *tzr);
static ast_primary *parse_primary(tokenizer *tzr) {
	ast_primary *prim = malloc(sizeof(ast_primary));
	token tkn;

again:
	switch ((tkn = advance(tzr)).kind) {
	case TK_ADD:
		goto again; // we ignore unary `+`s.

	case TK_LPAREN:
		prim->kind = AST_PAREN;
		if (!(prim->expr = parse_expression(tzr)))
			UNEXPECTED_TOKEN(tzr, peek(tzr));
		expect(tzr, TK_RPAREN);
		break;

	case TK_LBRACKET:
		prim->kind = AST_ARY;
		int cap = 4;
		prim->amnt = 0;
		prim->args = malloc(cap * sizeof(ast_expression *));
		while (!guard(tzr, TK_RBRACKET).kind) {
			if (prim->amnt == cap)
				prim->args = realloc(prim->args, (cap *= 2)*sizeof(ast_expression *));

			if (!(prim->args[prim->amnt++] = parse_expression(tzr)))
				UNEXPECTED_TOKEN(tzr, peek(tzr));

			if (!guard(tzr, TK_COMMA).kind) {
				expect(tzr, TK_RBRACKET);
				break;
			}
		}
		break;

	case TK_SUB:
	case TK_NOT:
		prim->kind = tkn.kind == TK_SUB ? AST_NEG : AST_NOT;
		if (!(prim->prim = parse_primary(tzr)))
			UNEXPECTED_TOKEN(tzr, peek(tzr));
		break;

	case TK_IDENT:
		prim->kind = AST_VAR;
		prim->ident = tkn.str;
		break;

	case TK_LITERAL:
		prim->kind = AST_LITERAL;
		prim->value = tkn.v;
		break;

	default:
		free(prim);
		return 0;
	}

	while ((tkn = peek(tzr)).kind == TK_LBRACKET || tkn.kind == TK_LPAREN) {
		ast_primary *prim2 = malloc(sizeof(ast_primary));
		prim2->prim = prim;
		prim = prim2;

		if (guard(tzr, TK_LBRACKET).kind) {
			prim->kind = AST_INDEX;
			if (!(prim->expr = parse_expression(tzr))) 
				UNEXPECTED_TOKEN(tzr, peek(tzr));
			expect(tzr, TK_RBRACKET);
			continue;
		}

		expect(tzr, TK_LPAREN);
		// parse function call
		prim->kind = AST_FNCALL;

		int cap = 4;
		prim->amnt = 0;
		prim->args = malloc(cap * sizeof(ast_expression *));
		while (!guard(tzr, TK_RPAREN).kind) {
			if (prim->amnt == cap)
				prim->args = realloc(prim->args, (cap *= 2)*sizeof(ast_expression *));

			if (!(prim->args[prim->amnt++] = parse_expression(tzr)))
				UNEXPECTED_TOKEN(tzr, peek(tzr));

			if (!guard(tzr, TK_COMMA).kind) {
				expect(tzr, TK_RPAREN);
				break;
			}
		}
	}

	return prim;
}

static ast_expression *parse_expression(tokenizer *tzr) {
	ast_expression *expr = malloc(sizeof(ast_expression));
	if (!(expr->prim = parse_primary(tzr))) {
		free(expr);
		return 0;
	}

	token tkn;
	switch ((tkn = advance(tzr)).kind) {
	case TK_ASSIGN:
		if (expr->prim->kind == AST_VAR) {
			expr->kind = AST_ASSIGN;
			char *name = expr->prim->ident;
			free(expr->prim);
			expr->name = name;
		} else if (expr->prim->kind == AST_INDEX) {
			expr->kind = AST_IDX_ASSIGN;
			expr->index = expr->prim->expr;
			ast_primary *prim = expr->prim->prim;
			free(expr->prim);
			expr->prim = prim;
		}

		if (!(expr->rhs = parse_expression(tzr)))
			UNEXPECTED_TOKEN(tzr, peek(tzr));
		break;

	case TK_ADD: case TK_SUB: case TK_MUL: case TK_DIV: case TK_MOD: case TK_NOT:
	case TK_LTH: case TK_GTH: case TK_LEQ: case TK_GEQ: case TK_EQL: case TK_NEQ:
		expr->kind = AST_BINOP;
		expr->binop = tkn.kind;
		if (!(expr->rhs = parse_expression(tzr)))
			UNEXPECTED_TOKEN(tzr, peek(tzr));
		break;

	default:
		unadvance(tzr, tkn);
		expr->kind = AST_PRIM;
	}

	return expr;
}

static ast_block *parse_block(tokenizer *tzr);

static ast_statement *parse_statement(tokenizer *tzr) {
	ast_statement *stmt = malloc(sizeof(ast_statement));
	token tkn;

	switch ((tkn = advance(tzr)).kind) {
	case TK_RETURN:
		stmt->kind = AST_RETURN;
		stmt->expr = parse_expression(tzr);
		expect(tzr, TK_SEMICOLON);
		break;

	case TK_CONTINUE:
	case TK_BREAK:
		stmt->kind = tkn.kind == TK_BREAK ? AST_BREAK : AST_CONTINUE;
		expect(tzr, TK_SEMICOLON);
		break;

	case TK_WHILE:
		stmt->kind = AST_WHILE;
		if (!(stmt->expr = parse_expression(tzr)))
			UNEXPECTED_TOKEN(tzr, peek(tzr));
		stmt->body = parse_block(tzr);
		break;

	case TK_IF:
		stmt->kind = AST_IF;
		if (!(stmt->expr = parse_expression(tzr)))
			UNEXPECTED_TOKEN(tzr, peek(tzr));

		stmt->body = parse_block(tzr);
		stmt->else_body = guard(tzr, TK_ELSE).kind ? parse_block(tzr) : 0;
		break;

	default:
		unadvance(tzr, tkn);
		if (!(stmt->expr = parse_expression(tzr))) {
			free(stmt);
			return 0;
		}
		stmt->kind = AST_EXPR;
		expect(tzr, TK_SEMICOLON);
	}

	return stmt;
}

static ast_block *parse_block(tokenizer *tzr) {
	ast_block *block = malloc(sizeof(ast_block));

	int cap = 4;
	block->amnt = 0;
	block->stmts = malloc(cap * sizeof(ast_statement*));

	expect(tzr, TK_LBRACE);
	while (!guard(tzr, TK_RBRACE).kind) {
		if (block->amnt == cap)
			block->stmts = realloc(block->stmts, (cap*=2) * sizeof(ast_statement*));

		// remove lonely semicolons
		while (guard(tzr, TK_SEMICOLON).kind);

		if (!(block->stmts[block->amnt++] = parse_statement(tzr)))
			UNEXPECTED_TOKEN(tzr, peek(tzr));
	}

	return block;
}

static ast_declaration *parse_global(tokenizer *tzr) {
	ast_declaration *decl = malloc(sizeof(ast_declaration));
	decl->kind = AST_GLOBAL;
	decl->name = expect(tzr, TK_IDENT).str;
	return decl;
}

static ast_declaration *parse_function(tokenizer *tzr) {
	ast_declaration *decl = malloc(sizeof(ast_declaration));
	decl->kind = AST_FUNCTION;
	decl->name = expect(tzr, TK_IDENT).str;
	expect(tzr, TK_LPAREN);

	int cap = 4;
	decl->argc = 0;
	decl->args = malloc(cap * sizeof(char*));
	while (!guard(tzr, TK_RPAREN).kind) {
		if (decl->argc == cap)
			decl->args = realloc(decl->args, (cap *= 2)*sizeof(char*));

		decl->args[decl->argc++] = expect(tzr, TK_IDENT).str;
		if (!guard(tzr, TK_COMMA).kind) {
			expect(tzr, TK_RPAREN);
			break;
		}
	}

	decl->block = parse_block(tzr);
	return decl;
}

ast_declaration *next_declaration(tokenizer *tzr) {
	token tkn;

	switch ((tkn = advance(tzr)).kind) {
	case TK_GLOBAL:
		return parse_global(tzr);
	case TK_FUNCTION:
		return parse_function(tzr);
	case TK_EOF:
		return 0;
	default:
		UNEXPECTED_TOKEN(tzr, tkn);
	}
}
