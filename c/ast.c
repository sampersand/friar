#include "ast.h"
#include "token.h"
#include "value.h"
#include <stdlib.h>
#include <assert.h>

void free_ast_primary(ast_primary *primary) {
	switch (primary->kind) {
	case AST_PRIMARY_PAREN:
		free_ast_expression(primary->paren.expression);
		break;

	case AST_PRIMARY_INDEX:
		free_ast_primary(primary->index.source);
		free_ast_expression(primary->index.index);
		break;

	case AST_PRIMARY_FUNCTION_CALL:
		free_ast_primary(primary->function_call.function);

		for (unsigned i = 0; i < primary->function_call.number_of_arguments; i++)
			free_ast_expression(primary->function_call.arguments[i]);
		free(primary->function_call.arguments);

		break;

	case AST_PRIMARY_UNARY_OPERATOR:
		free_ast_primary(primary->unary_operator.primary);
		break;

	case AST_PRIMARY_ARRAY_LITERAL:
		for (unsigned i = 0; i < primary->array_literal.length; i++)
			free_ast_expression(primary->array_literal.elements[i]);
		free(primary->array_literal.elements);
		break;

	case AST_PRIMARY_VARIABLE:
		free(primary->variable.name);
		break;

	case AST_PRIMARY_LITERAL:
		free_value(primary->literal.val);
		break;
	}

	free(primary);
}

void free_ast_expression(ast_expression *expression) {
	switch (expression->kind) {
	case AST_EXPRESSION_ASSIGN:
		free(expression->assign.name);
		free_ast_expression(expression->assign.value);
		break;

	case AST_EXPRESSION_INDEX_ASSIGN:
		free_ast_primary(expression->index_assign.source);
		free_ast_expression(expression->index_assign.index);
		free_ast_expression(expression->index_assign.value);
		break;

	case AST_EXPRESSION_BINARY_OPERATOR:
		free_ast_expression(expression->binary_operator.lhs);
		free_ast_expression(expression->binary_operator.rhs);
		break;

	case AST_EXPRESSION_PRIMARY:
		free_ast_primary(expression->primary);
		break;
	}
}

void free_ast_statement(ast_statement *statement) {
	switch (statement->kind) {
	case AST_STATEMENT_RETURN:
		if (statement->return_.expression != NULL)
			free_ast_expression(statement->return_.expression);
		break;

	case AST_STATEMENT_IF:
		free_ast_expression(statement->if_.condition);
		free_ast_block(statement->if_.if_true);
		if (statement->if_.if_false != NULL)
			free_ast_block(statement->if_.if_false);
		break;

	case AST_STATEMENT_WHILE:
		free_ast_expression(statement->while_.condition);
		free_ast_block(statement->while_.body);
		break;

	case AST_STATEMENT_BREAK:
		break;

	case AST_STATEMENT_CONTINUE:
		break;

	case AST_STATEMENT_EXPRESSION:
		free_ast_expression(statement->expression);
		break;
	}

	free(statement);
};

void free_ast_block(ast_block *block) {
	for (unsigned i = 0; i < block->number_of_statements; i++)
		free_ast_statement(block->statements[i]);

	free(block->statements);
	free(block);
}

void free_ast_declaration(ast_declaration *declaration) {
	switch (declaration->kind) {
	case AST_DECLARATION_GLOBAL:
		free(declaration->global.name);
		break;

	case AST_DECLARATION_FUNCTION:
		free(declaration->function.name);

		for (unsigned i = 0; i < declaration->function.number_of_arguments; i++)
			free(declaration->function.argument_names[i]);
		free(declaration->function.argument_names);

		free_ast_block(declaration->function.body);
		break;
	}
	free(declaration);
}
