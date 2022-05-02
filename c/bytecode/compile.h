#pragma once

#include "base/ast.h"
#include "base/environment.h"

typedef struct {
	map globals;
} compiler;

void init_compiler(compiler *compiler);
void compile_declaration(compiler *comp, ast_declaration *declaration);
