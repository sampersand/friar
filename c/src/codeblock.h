#pragma once

#include "bytecode.h"
#include "valuedefn.h"

#define CODEBLOCK_RETURN_LOCAL 0

typedef struct codeblock {
	unsigned code_length, number_of_locals, number_of_constants;
	bytecode *code;
	value *constants;
} codeblock;

value run_codeblock(const codeblock *block, unsigned number_of_arguments, const value *arguments);
void free_codeblock(codeblock *block);
