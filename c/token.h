#pragma once
#include <stdio.h>
#include "value.h"

typedef enum {
	TK_EOF = '\0',

	TK_LITERAL, TK_IDENT,

	TK_GLOBAL, TK_FUNCTION,
	TK_IF, TK_ELSE, TK_WHILE, TK_BREAK, TK_CONTINUE, TK_RETURN,

	// not terrific practice, but it makes parsing easier
	TK_LPAREN = '(',
	TK_RPAREN = ')',
	TK_LBRACKET = '[',
	TK_RBRACKET = ']',
	TK_LBRACE = '{',
	TK_RBRACE = '}',
	TK_ASSIGN = '=',
	TK_COMMA = ',',
	TK_SEMICOLON = ';',
	TK_ADD = '+',
	TK_SUB = '-',
	TK_MUL = '*',
	TK_DIV = '/',
	TK_MOD = '%',
	TK_NOT = '!',
	TK_LTH = '<',
	TK_GTH = '>',

	TK_LEQ = TK_LTH + 0x80,
	TK_GEQ = TK_LEQ + 0x80,
	TK_EQL = TK_ASSIGN + 0x80,
	TK_NEQ = TK_NOT + 0x80
} token_kind; 

typedef struct token {
	token_kind kind;
	union {
		value v;
		char *str;
	};
} token;

typedef struct tokenizer {
	const char *stream;
	int lineno;
	token prev;
} tokenizer;

tokenizer new_tokenizer(const char *stream);
token next_token(tokenizer *);
void dump_token(FILE *out, token tkn);
