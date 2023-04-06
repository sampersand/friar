#pragma once
#include <stdio.h>
#include "value/defn.h"
#include "value/number.h"
#include "value/string_.h"

typedef enum {
	// Indicates that the token isn't actually a token.
	// This is used for things like marking the end of a stream.
	TOKEN_KIND_UNDEFINED,


	// Literal values within source code.
	TOKEN_KIND_TRUE, TOKEN_KIND_FALSE, TOKEN_KIND_NULL, 

	// These make use of the `union` within the `token`.
	TOKEN_KIND_NUMBER ,TOKEN_KIND_STRING, TOKEN_KIND_IDENTIFIER,

	// Keywords
	TOKEN_KIND_IMPORT,
	TOKEN_KIND_GLOBAL, TOKEN_KIND_FUNCTION, TOKEN_KIND_LOCAL,
	TOKEN_KIND_IF, TOKEN_KIND_ELSE,
	TOKEN_KIND_WHILE, TOKEN_KIND_FOR, TOKEN_KIND_BREAK, TOKEN_KIND_CONTINUE,
	TOKEN_KIND_RETURN,

	// Punctuation
	TOKEN_KIND_LPAREN, TOKEN_KIND_RPAREN,
	TOKEN_KIND_LBRACKET, TOKEN_KIND_RBRACKET,
	TOKEN_KIND_LBRACE, TOKEN_KIND_RBRACE,
	TOKEN_KIND_COMMA, TOKEN_KIND_SEMICOLON,
	TOKEN_KIND_ASSIGN,

	// Math operators and their assignment overloads.
	TOKEN_KIND_ADD, TOKEN_KIND_SUBTRACT,
	TOKEN_KIND_MULTIPLY, TOKEN_KIND_DIVIDE, TOKEN_KIND_MODULO,

	// Math assignment operators
	TOKEN_KIND_ADD_ASSIGN, TOKEN_KIND_SUBTRACT_ASSIGN,
	TOKEN_KIND_MULTIPLY_ASSIGN, TOKEN_KIND_DIVIDE_ASSIGN, TOKEN_KIND_MODULO_ASSIGN,

	// Short circuit operators
	TOKEN_KIND_AND_AND, TOKEN_KIND_OR_OR,

	// Logical operators
	TOKEN_KIND_NOT,
	TOKEN_KIND_EQUAL, TOKEN_KIND_NOT_EQUAL,
	TOKEN_KIND_LESS_THAN, TOKEN_KIND_LESS_THAN_OR_EQUAL,
	TOKEN_KIND_GREATER_THAN, TOKEN_KIND_GREATER_THAN_OR_EQUAL,
} token_kind;

/** A source code token.
 * There are few different standard ways of representing tokens in language design. The simplest is
 */
typedef struct {
	token_kind kind;
	union {
		number num;
		string *str;
		char *identifier;
	};
} token;

typedef struct {
	const char *stream, *filename;
	unsigned line_number;
	token prev;
} tokenizer;

tokenizer new_tokenizer(const char *filename, const char *stream);
token next_token(tokenizer *tzr);
void dump_token(FILE *out, token tkn);
