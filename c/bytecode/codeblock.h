#pragma once

#include "bytecode/bytecode.h"
#include "base/valuedefn.h"
#include "base/environment.h"

#define CODEBLOCK_RETURN_LOCAL 0
typedef struct codeblock {
	unsigned code_length, number_of_locals, number_of_constants;
	bytecode *code;
	value *constants;
} codeblock;

value run_codeblock(const codeblock *block, environment *env);
void free_codeblock(codeblock *block);
