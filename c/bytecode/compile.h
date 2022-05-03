#pragma once

#include "base/ast.h"
#include "base/environment.h"
#include "bytecode/globals.h"

typedef struct {
	global_variables *globals;
	// If you wanted to keep track of type declarations (or even function defns) separately,
	// you could add them to this type.
} compiler;

void init_compiler(compiler *compiler);
void compile_declaration(compiler *comp, ast_declaration *declaration);