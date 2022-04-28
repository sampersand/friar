#pragma once
#include <stdio.h>
#include <assert.h>
#include "shared.h"

/*
00...000 = VFALSE
00...001 = VNULL
00...010 = VTRUE
XX...100 = number
XX...000 = string
XX...001 = function
XX...010 = ary
*/

typedef long long value;
typedef struct {
	int cap, len;
	value *eles;
} array;

#define VFALSE 0
#define VNULL 1
#define VTRUE 2
#define VUNDEF 3

static inline value ary2value(array *a) {
	assert(((size_t) a & 7) == 0);
	return (value) a | 2;
}
static inline array *value2ary(value v) {
	assert((v & 7) == 2);
	return (array *) (v & ~2);
}

static inline value num2value(long long n) {
	return (n << 3) | 4;
}

static inline long long value2num(value v) {
	return (long long) v >> 3;
}

static inline value str2value(char *s) {
	assert(((size_t) s & 7) == 0);
	return (long long) s;
}
static inline char *value2str(value v) {
	assert((v & 7) == 0);
	return (char *) v;
}

static inline enum { V_INT, V_STR, V_BOOL, V_NULL, V_ARY, V_FUNC } classify(value v) {
	if (v == VNULL) return V_NULL;
	if (v == VTRUE || v==VFALSE) return V_BOOL;
	if ((v & 7) == 4) return V_INT;
	if ((v & 7) == 0) return V_STR;
	if ((v & 7) == 2) return V_ARY;
	if ((v & 7) == 1) return V_FUNC;
	die("unknown value kind %llx", v);
}

static inline int value2bool(value v) {
	return v != VNULL & v != VFALSE;
}

static inline int is_number(value v) {
	return (v & 7) == 4;
}

void dump_value(FILE *out, value v);

struct ast_block;
value new_function(char *name, int argc, char **argv, struct ast_block *block);
void index_assign(value ary, value idx, value val);
value index_into(value ary, value idx);
struct _env;
value call_value(value v, int argc, value *argv, struct _env *e);

