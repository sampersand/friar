#pragma once

#include "bytecode.h"
#include "valuedefn.h"
#include "environment.h"

#define CODEBLOCK_RETURN_LOCAL 0
typedef struct codeblock {
	unsigned code_length, number_of_locals, number_of_constants;
	bytecode *code;
	value *constants;
} codeblock;

value run_codeblock(const codeblock *block, unsigned argc, const value *argv, environment *env);
void free_codeblock(codeblock *block);
