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
		.prev = (token) { .kind = TK_UNDEFINED }
	};
}

#define parse_error(tzr, msg, ...) (die(\
	"invalid syntax at %d: " msg, tzr->lineno, __VA_ARGS__))

static char peek(const tokenizer *tzr) {
	return tzr->stream[0];
}

static void advance(tokenizer *tzr) {
	if (peek(tzr) == '\n')
		tzr->lineno++;

	tzr->stream++;
}

static char peek_advance(tokenizer *tzr) {
	char peeked = peek(tzr);
	advance(tzr);
	return peeked;
}

static token parse_number(tokenizer *tzr) {
	number num = 0;
	char c;

	while (isdigit(c = peek(tzr))) {
		num = num * 10 + (c - '0');
		advance(tzr);
	}

	if (isalnum(c) || c == '_')
		parse_error(tzr, "bad character '%c' after integer literal", c);

	return (token) { .kind = TK_LITERAL, .val = new_number_value(num) };
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
	if (!strncmp(start, "true", length)) return (token) { .kind = TK_LITERAL, .val = VTRUE };
	if (!strncmp(start, "false", length)) return (token) { .kind = TK_LITERAL, .val = VFALSE };
	if (!strncmp(start, "null", length)) return (token) { .kind = TK_LITERAL, .val = VNULL };
	if (!strncmp(start, "global", length)) return (token) { .kind = TK_GLOBAL };
	if (!strncmp(start, "function", length)) return (token) { .kind = TK_FUNCTION };
	if (!strncmp(start, "if", length)) return (token) { .kind = TK_IF };
	if (!strncmp(start, "else", length)) return (token) { .kind = TK_ELSE };
	if (!strncmp(start, "while", length)) return (token) { .kind = TK_WHILE };
	if (!strncmp(start, "break", length)) return (token) { .kind = TK_BREAK };
	if (!strncmp(start, "continue", length)) return (token) { .kind = TK_CONTINUE };
	if (!strncmp(start, "return", length)) return (token) { .kind = TK_RETURN };

	// it's a normal identifier, retunr that.
	return (token) { .kind = TK_IDENTIFIER, .str = strndup(start, length) };
}

static int parse_hex(tokenizer *tzr, char c) {
	if (isdigit(c))
		return c - '0';

	if ('a' <= c && c <= 'f')
		return (c - 'a') + 10;

	if ('A' <= c && c <= 'F')
		return (c - 'A') + 10;

	parse_error(tzr, "unknown hex digit '%c'", c);
}

static char get_escape_char(tokenizer *tzr) {
	char c = peek_advance(tzr);
	switch (c) {
	case '\'':
	case '\"':
	case '\\': return c;
	case 'n': return '\n';
	case 't': return '\t';
	case 'r': return '\r';
	case 'f': return '\f';
	case '0': return '\0';

	case 'x':
		if (tzr->stream[0] == '\0' || tzr->stream[1] == '\0')
			parse_error(tzr, "unterminated '\\x' sequence encountered %s", "");

		char upper_nibble = peek_advance(tzr);
		char lower_nibble = peek_advance(tzr);
		
		return (parse_hex(tzr, upper_nibble) << 4) + parse_hex(tzr, lower_nibble);

	default:
		parse_error(tzr, "unknown escape character '%c'", c);
	}
}

static token parse_string(tokenizer *tzr) {
	char quote = peek_advance(tzr);
	assert(quote == '\'' || quote == '\"');

	unsigned length = 0;
	unsigned capacity = 8;
	char *str = xmalloc(capacity + 1); // `+1` for the trailing `\0`.

	int starting_line = tzr->lineno;

	char c;
	while ((c = peek_advance(tzr)) != quote) {
		if (c == '\0')
			parse_error(tzr, "unterminated quote encountered starting on line %d", starting_line);

		if (c == '\\')
			c = get_escape_char(tzr);

		if (length == capacity) {
			capacity *= 2;
			str = xrealloc(str, capacity + 1);
		}

		str[length++] = c;
	}

	str = xrealloc(str, length + 1);
	str[length] = '\0';

	return (token) {
		.kind = TK_LITERAL,
		.val = new_string_value(new_string2(str, length))
	};
}

static void strip_leading_whitespace_and_comments(tokenizer *tzr) {
	char c;

	while ((c = peek(tzr)) != '\0') {
		// if `c` is a whitespace character, then just discard it.
		if (isspace(c)) {
			advance(tzr);
			continue;
		}

		// briar only has line comments
		if (c == '/' && tzr->stream[1] == '/') {
			while (c != '\0' && c != '\n')
				c = peek_advance(tzr);

			continue;
		}

		break;
	}
}

static bool advance_if_equal(tokenizer *tzr) {
	if (peek(tzr) == '=') {
		advance(tzr);
		return true;
	}

	return false;
}

token next_token(tokenizer *tzr) {
	strip_leading_whitespace_and_comments(tzr);
	char c = peek(tzr);

	if (c == '\0')
		return (token) { .kind = TK_UNDEFINED };

	if (isdigit(c))
		return parse_number(tzr);

	if (isalpha(c) || c == '_')
		return parse_identifier(tzr);

	if (c == '\'' || c == '\"')
		return parse_string(tzr);

	// We don't want to advance before calling the previous parser kinds.
	advance(tzr);

	switch (c) {
	// Simple tokens
	case '(': return (token) { .kind = TK_LPAREN };
 	case ')': return (token) { .kind = TK_RPAREN };
	case '[': return (token) { .kind = TK_LBRACKET };
	case ']': return (token) { .kind = TK_RBRACKET };
	case '{': return (token) { .kind = TK_LBRACE };
	case '}': return (token) { .kind = TK_RBRACE };
	case ',': return (token) { .kind = TK_COMMA };
	case ';': return (token) { .kind = TK_SEMICOLON };

	// compound tokens
	case '&':
		if ((c = peek_advance(tzr)) != '&')
			parse_error(tzr, "only `&&` is recognized, not `&%c`", c);
		return (token) { .kind = TK_AND_AND };

	case '|':
		if ((c = peek_advance(tzr)) != '|')
			parse_error(tzr, "only `||` is recognized, not `|%c`", c);
		return (token) { .kind = TK_OR_OR };

	case '=':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_EQUAL : TK_ASSIGN
		};

	case '!':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_NOT_EQUAL : TK_NOT
		};

	case '<':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_LESS_THAN_OR_EQUAL : TK_LESS_THAN
		};

	case '>':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_GREATER_THAN_OR_EQUAL : TK_GREATER_THAN
		};

	case '+':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_ADD_ASSIGN : TK_ADD
		};

	case '-':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_SUBTRACT_ASSIGN : TK_SUBTRACT
		};

	case '*':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_MULTIPLY_ASSIGN : TK_MULTIPLY
		};

	case '/':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_DIVIDE_ASSIGN : TK_DIVIDE
		};

	case '%':
		return (token) {
			.kind = advance_if_equal(tzr) ? TK_MODULO_ASSIGN : TK_MODULO
		};

	default:
		parse_error(tzr, "unknown token start: '%c' (%02x)", c, c);
	}
}

void dump_token(FILE *out, token tkn) {
	switch(tkn.kind) {
	case TK_UNDEFINED: fputs("UNDEF", out); break;

	case TK_LITERAL: dump_value(out, tkn.val); break;
	case TK_IDENTIFIER: fprintf(out, "Identifier(%s)\n", tkn.str); break;

	case TK_GLOBAL: fputs("Keyword(global)", out); break;
	case TK_FUNCTION: fputs("Keyword(function)", out); break;
	case TK_IF: fputs("Keyword(if)", out); break;
	case TK_ELSE: fputs("Keyword(else)", out); break;
	case TK_WHILE: fputs("Keyword(while)", out); break;
	case TK_BREAK: fputs("Keyword(break)", out); break;
	case TK_CONTINUE: fputs("Keyword(continue)", out); break;
	case TK_RETURN: fputs("Keyword(return)", out); break;

	case TK_LPAREN: fputs("Token('(')", out); break;
	case TK_RPAREN: fputs("Token(')')", out); break;
	case TK_LBRACKET: fputs("Token('[')", out); break;
	case TK_RBRACKET: fputs("Token(']')", out); break;
	case TK_LBRACE: fputs("Token('{')", out); break;
	case TK_RBRACE: fputs("Token('}')", out); break;
	case TK_COMMA: fputs("Token(',')", out); break;
	case TK_SEMICOLON: fputs("Token(';')", out); break;
	case TK_ASSIGN: fputs("Token('=')", out); break;

	case TK_ADD: fputs("Token('+')", out); break;
	case TK_SUBTRACT: fputs("Token('-')", out); break;
	case TK_MULTIPLY: fputs("Token('*')", out); break;
	case TK_DIVIDE: fputs("Token('/')", out); break;
	case TK_MODULO: fputs("Token('%')", out); break;

	case TK_ADD_ASSIGN: fputs("Token('+=')", out); break;
	case TK_SUBTRACT_ASSIGN: fputs("Token('-=')", out); break;
	case TK_MULTIPLY_ASSIGN: fputs("Token('*=')", out); break;
	case TK_DIVIDE_ASSIGN: fputs("Token('/=')", out); break;
	case TK_MODULO_ASSIGN: fputs("Token('%=')", out); break;

	case TK_AND_AND: fputs("Token('&&')", out); break;
	case TK_OR_OR: fputs("Token('||')", out); break;

	case TK_NOT: fputs("Token('!')", out); break;
	case TK_EQUAL: fputs("Token('==')", out); break;
	case TK_NOT_EQUAL: fputs("Token('!=')", out); break;
	case TK_LESS_THAN: fputs("Token('<')", out); break;
	case TK_LESS_THAN_OR_EQUAL: fputs("Token('<=')", out); break;
	case TK_GREATER_THAN: fputs("Token('>')", out); break;
	case TK_GREATER_THAN_OR_EQUAL: fputs("Token('>=')", out); break;
	}
}
