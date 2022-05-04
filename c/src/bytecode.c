#include "bytecode.h"

const char *opcode_repr(opcode op) {
	switch (op) {
	case OPCODE_MOVE:          return "OPCODE_MOVE";
	case OPCODE_ARRAY_LITERAL: return "OPCODE_ARRAY_LITERAL";

	case OPCODE_LOAD_CONSTANT:         return "OPCODE_LOAD_CONSTANT";
	case OPCODE_LOAD_GLOBAL_VARIABLE:  return "OPCODE_LOAD_GLOBAL_VARIABLE";
	case OPCODE_STORE_GLOBAL_VARIABLE: return "OPCODE_STORE_GLOBAL_VARIABLE";

	case OPCODE_JUMP:          return "OPCODE_JUMP";
	case OPCODE_JUMP_IF_TRUE:  return "OPCODE_JUMP_IF_TRUE";
	case OPCODE_JUMP_IF_FALSE: return "OPCODE_JUMP_IF_FALSE";
	case OPCODE_CALL:          return "OPCODE_CALL";
	case OPCODE_RETURN:        return "OPCODE_RETURN";

	case OPCODE_NOT:      return "OPCODE_NOT";
	case OPCODE_NEGATE:   return "OPCODE_NEGATE";
	case OPCODE_ADD:      return "OPCODE_ADD";
	case OPCODE_SUBTRACT: return "OPCODE_SUBTRACT";
	case OPCODE_MULTIPLY: return "OPCODE_MULTIPLY";
	case OPCODE_DIVIDE:   return "OPCODE_DIVIDE";
	case OPCODE_MODULO:   return "OPCODE_MODULO";
	case OPCODE_EQUAL:                 return "OPCODE_EQUAL";
	case OPCODE_NOT_EQUAL:             return "OPCODE_NOT_EQUAL";
	case OPCODE_LESS_THAN:             return "OPCODE_LESS_THAN";
	case OPCODE_LESS_THAN_OR_EQUAL:    return "OPCODE_LESS_THAN_OR_EQUAL";
	case OPCODE_GREATER_THAN:          return "OPCODE_GREATER_THAN";
	case OPCODE_GREATER_THAN_OR_EQUAL: return "OPCODE_GREATER_THAN_OR_EQUAL";

	case OPCODE_INDEX:       return "OPCODE_INDEX";
	case OPCODE_INDEX_ASSIGN: return "OPCODE_INDEX_ASSIGN";
	}
}
