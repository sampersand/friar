#include "token.h"
#include "shared.h"
#include "value.h"

#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

tokenizer new_tokenizer(const char *stream) {
	return (tokenizer) {
		.stream = stream,
		.lineno = 1,
		.prev = (token) { .kind = TK_EOF }
	};
}

#define parse_error(tzr, msg, ...) (die(\
	"invalid syntax at %d: " msg, tzr->lineno, __VA_ARGS__))

static char peek(tokenizer *tzr) {
	return tzr->stream[0];
}

static void advance(tokenizer *tzr) {
	if (peek(tzr) == '\n')
		tzr->lineno++;

	tzr->stream++;
}

static token parse_integer(tokenizer *tzr) {
	number num = 0;
	char c;

	while (isdigit(c = peek(tzr))) {
		num = num * 10 + (c - '0');
		advance(tzr);
	}

	if (isalnum(c) || c == '_')
		parse_error(tzr, "bad character '%c' after integer literal", c);

	return (token) { .kind = TK_LITERAL, .v = new_number_value(num) };
}

static bool isalnum_or_underscore(char c) {
	return isalnum(c) || c == '_';
}

static token parse_identifier(tokenizer *tzr) {
	const char *start = tzr->stream;

	// find the length of the identifier.
	while (isalnum_or_underscore(peek(tzr)))
		advance(tzr);
	unsigned length = tzr->stream - start;

	// check for predefined identifiers
	if (!strncmp(start, "true", length)) return (token) { .kind = TK_LITERAL, .v = VTRUE };
	if (!strncmp(start, "false", length)) return (token) { .kind = TK_LITERAL, .v = VFALSE };
	if (!strncmp(start, "null", length)) return (token) { .kind = TK_LITERAL, .v = VNULL };
	if (!strncmp(start, "global", length)) return (token) { .kind = TK_GLOBAL };
	if (!strncmp(start, "function", length)) return (token) { .kind = TK_FUNCTION };
	if (!strncmp(start, "if", length)) return (token) { .kind = TK_IF };
	if (!strncmp(start, "else", length)) return (token) { .kind = TK_ELSE };
	if (!strncmp(start, "while", length)) return (token) { .kind = TK_WHILE };
	if (!strncmp(start, "break", length)) return (token) { .kind = TK_BREAK };
	if (!strncmp(start, "continue", length)) return (token) { .kind = TK_CONTINUE };
	if (!strncmp(start, "return", length)) return (token) { .kind = TK_RETURN };

	// it's a normal identifier, retunr that.
	return (token) { .kind = TK_IDENT, .str = strndup(start, length) };
}

static int parse_hex(tokenizer *tzr, char c) {
	if (isdigit(c))
		return c - '0';

	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;

	if ('A' <= c && c <= 'F')
		return c - 'F' + 10;

	parse_error(tzr, "unknown hex digit '%c'", c);
}

static token parse_string(tokenizer *tzr) {
	char quote = peek(tzr);
	advance(tzr);
	assert(quote == '\'' || quote == '\"');

	const char *start = tzr->stream;
	bool was_anything_escaped = false;
	int starting_line = tzr->lineno;

	char c;
	do {
		c = peek(tzr);

		if (c == '\0')
			parse_error(tzr, "unterminated quote encountered started on %d", starting_line);

		if (c == '\\') {
			was_anything_escaped = true;
			advance(tzr);

			if (peek(tzr) == '\0')
				parse_error(tzr, "unterminated escape sequence of string started on %d", starting_line);
		}

		advance(tzr);
	} while (c != quote);

	// We need the `- 1` as it'll exclude the closing quote.
	unsigned length = tzr->stream - start - 1;

	// simple case, just return the original string.
	if (!was_anything_escaped) {
		return (token) {
			.kind = TK_LITERAL,
			.v = new_string_value(new_string2(strndup(start, length), length))
		};
	}

	// well, something was escaped, so we now need to deal with that.
	char *return_string = malloc(length); // note not `+1`, as we're removing at least 1 slash.
	int i = 0, stridx = 0;
	char *str = return_string;

	while (i < length) {
		if (start[i] != '\\') {
			str[stridx++] = start[i++];
			continue;
		}

		char c = start[++i];

		if (quote == '\'') {
			if (c != '\\' && c != '\"' && c != '\'')
				str[stridx++] = '\\';
		} else {
			switch (c) {
			case '\'': case '\"': case '\\': break;
			case 'n': c = '\n'; break;
			case 't': c = '\t'; break;
			case 'r': c = '\r'; break;
			case 'f': c = '\f'; break;
			case '0': c = '\0'; break;
			case 'x':
				i += 3;
				c = (parse_hex(tzr, start[i-2]) << 4) + parse_hex(tzr, start[i-1]);
				break;
			default:
				parse_error(tzr, "unknown escape character '%c'", c);
			}
		}

		str[stridx++] = c;
		i++;
	}

	str[stridx] = '\0';
	return (token) { .kind = TK_LITERAL, .v = new_string_value(new_string1(str)) };
}

token next_token(tokenizer *tzr) {
	char c;

	// Strip whitespace and comments.
	for (; (c = peek(tzr)); advance(tzr)) {
		if (c == '#')
			do {
				advance(tzr);
			} while ((c = peek(tzr)) && c != '\n');

		if (!isspace(c))
			break;
	}

	// For simple tokens, just return them.
	switch (c) {
	case '=': case '!': case '<': case '>':
		if (tzr->stream[1] == '=')
			tzr->stream++, c += 0x80;

		// fallthrough
	case '(': case ')': case '[': case ']': case '{': case '}':
	case '+': case '-': case '*': case '/': case '%':
	case ',': case ';': 
		advance(tzr);
		// fallthru

	case '\0':
		return (token) { .kind = c };
	}

	// for more complicated ones, defer to their functions.
	if (isdigit(c)) return parse_integer(tzr);
	if (isalpha(c) || c == '_') return parse_identifier(tzr);
	if (c == '\'' || c == '\"') return parse_string(tzr);

	parse_error(tzr, "unknown token start: '%c'", c);
}


void dump_token(FILE *out, token tkn) {
	switch(tkn.kind) {
	case TK_EOF: fprintf(out, "EOF\n"); break;
	case TK_LITERAL: dump_value(out, tkn.v); break;
	case TK_IDENT: fprintf(out, "Ident(%s)\n", tkn.str); break;
	case TK_GLOBAL: fprintf(out, "Token(global)\n"); break;
	case TK_FUNCTION: fprintf(out, "Token(function)\n"); break;
	case TK_IF: fprintf(out, "Token(if)\n"); break;
	case TK_ELSE: fprintf(out, "Token(else)\n"); break;
	case TK_WHILE: fprintf(out, "Token(while)\n"); break;
	case TK_BREAK: fprintf(out, "Token(break)\n"); break;
	case TK_CONTINUE: fprintf(out, "Token(continue)\n"); break;
	case TK_RETURN: fprintf(out, "Token(return)\n"); break;
	case TK_LPAREN: fprintf(out, "Token[(]\n"); break;
	case TK_RPAREN: fprintf(out, "Token[)]\n"); break;
	case TK_LBRACKET: fprintf(out, "Token([)\n"); break;
	case TK_RBRACKET: fprintf(out, "Token(])\n"); break;
	case TK_LBRACE: fprintf(out, "Token({)\n"); break;
	case TK_RBRACE: fprintf(out, "Token(})\n"); break;
	case TK_ASSIGN: fprintf(out, "Token(=)\n"); break;
	case TK_COMMA: fprintf(out, "Token(,)\n"); break;
	case TK_SEMICOLON: fprintf(out, "Token(;)\n"); break;
	case TK_ADD: fprintf(out, "Token(+)\n"); break;
	case TK_SUB: fprintf(out, "Token(-)\n"); break;
	case TK_MUL: fprintf(out, "Token(*)\n"); break;
	case TK_DIV: fprintf(out, "Token(/)\n"); break;
	case TK_MOD: fprintf(out, "Token(%%)\n"); break;
	case TK_NOT: fprintf(out, "Token(!)\n"); break;
	case TK_LTH: fprintf(out, "Token(<)\n"); break;
	case TK_GTH: fprintf(out, "Token(>)\n"); break;
	case TK_LEQ: fprintf(out, "Token(<=)\n"); break;
	case TK_GEQ: fprintf(out, "Token(>=)\n"); break;
	case TK_EQL: fprintf(out, "Token(==)\n"); break;
	case TK_NEQ: fprintf(out, "Token(!=)\n"); break;
	default: fprintf(out, "Token(<%d>)\n", tkn.kind); break;
	}
}
