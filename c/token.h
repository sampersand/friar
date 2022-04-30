#pragma once
#include <stdio.h>
#include "valuedefn.h"

typedef enum {
	// Indicates that the token isn't actually a token.
	// This is used for things like marking the end of a stream.
	TK_UNDEFINED = 0,

	// These two make use of the `union` within the `token`.
	TK_LITERAL, TK_IDENTIFIER,

	// Keywords
	TK_GLOBAL, TK_FUNCTION,
	TK_IF, TK_ELSE,
	TK_WHILE, TK_BREAK, TK_CONTINUE,
	TK_RETURN,

	// Punctuation
	TK_LPAREN, TK_RPAREN,
	TK_LBRACKET, TK_RBRACKET,
	TK_LBRACE, TK_RBRACE,
	TK_COMMA, TK_SEMICOLON,
	TK_ASSIGN, 

	// Math operators and their assignment overloads.
	TK_ADD, TK_SUBTRACT,
	TK_MULTIPLY, TK_DIVIDE, TK_MODULO,

	// Math assignment operators
	TK_ADD_ASSIGN, TK_SUBTRACT_ASSIGN,
	TK_MULTIPLY_ASSIGN, TK_DIVIDE_ASSIGN, TK_MODULO_ASSIGN,

	// Short circuit operators
	TK_AND_AND, TK_OR_OR,

	// Logical operators
	TK_NOT, 
	TK_EQUAL, TK_NOT_EQUAL,
	TK_LESS_THAN, TK_LESS_THAN_OR_EQUAL,
	TK_GREATER_THAN, TK_GREATER_THAN_OR_EQUAL,
} token_kind; 

typedef struct {
	token_kind kind;
	union {
		value val;
		char *str;
	};
} token;

typedef struct {
	const char *stream;
	int lineno;
	token prev;
} tokenizer;

tokenizer new_tokenizer(const char *stream);
token next_token(tokenizer *tzr);
void dump_token(FILE *out, token tkn);
