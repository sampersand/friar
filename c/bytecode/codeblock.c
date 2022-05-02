#include "bytecode/codeblock.h"
#include "base/shared.h"
#include "base/value.h"

typedef struct {
	const codeblock *block;
	unsigned instruction_pointer;
	environment *env;
	value* locals;
} virtual_machine;

void free_codeblock(codeblock *block) {
	for (unsigned i = 0; i < block->number_of_constants; i++)
		free_value(block->constants[i]);
	free(block->constants);
	free(block->code);
	free(block);
}

static value run_vm(virtual_machine *vm);

value run_codeblock(const codeblock *block, environment *env) {
	virtual_machine vm;

	vm.block = block;
	vm.instruction_pointer = 0;
	vm.env = env;

	value locals[block->number_of_locals];
	vm.locals = locals;
	for (unsigned i = 0; i < block->number_of_locals; i++)
		vm.locals[i] = VUNDEF;

	value return_value = run_vm(&vm);

	for (unsigned i = 0; i < block->number_of_locals; i++) {
		if (locals[i] != VUNDEF)
			free_value(locals[i]);
	}

	return return_value;
}

static unsigned next_count(virtual_machine *vm) {
	unsigned count = vm->block->code[vm->instruction_pointer].count;
	LOG("vm[% 3d] = count(%d)", vm->instruction_pointer, count);
	vm->instruction_pointer++;
	return count;
}

static opcode next_opcode(virtual_machine *vm) {
	opcode op = vm->block->code[vm->instruction_pointer].op;
	LOG("vm[% 3d] = op(%s)", vm->instruction_pointer, opcode_repr(op));
	vm->instruction_pointer++;
	return op;
}

static value next_local(virtual_machine *vm) {
	unsigned count = vm->block->code[vm->instruction_pointer].count;
	value local = vm->locals[count];

	LOGN("vm[% 3d] = local(%d) {", vm->instruction_pointer, count);
#ifndef NDEBUG
	dump_value(stdout, local);
	puts("}");
#endif

	vm->instruction_pointer++;
	return local;
}

static void set_next_local(virtual_machine *vm, value val) {
	vm->locals[next_count(vm)] = val;
}

static value run_vm(virtual_machine *vm) {
	while (vm->instruction_pointer < vm->block->code_length) {
		switch (next_opcode(vm)) {
		case OPCODE_MOV:
			set_next_local(vm, next_local(vm));
			break;

		case OPCODE_ARRAY_LITERAL: {
			unsigned count = next_count(vm);
			array *ary = allocate_array(count);

			for (unsigned i = 0; i < count; i++)
				push_array(ary, next_local(vm));

			set_next_local(vm, new_array_value(ary));
			break;
		}

		case OPCODE_LOAD_CONSTANT: {
			value constant = vm->block->constants[next_count(vm)];
		#ifndef NDEBUG
			LOGN("constant=");
			dump_value(stdout, constant);
			puts("");
		#endif
			set_next_local(vm, constant);
			break;
		}

		case OPCODE_LOAD_GLOBAL_VARIABLE:
			// set_next_local(lookup_global_var
		case OPCODE_STORE_GLOBAL_VARIABLE:
			die("todo");


		case OPCODE_JUMP_IF_TRUE: {
			value condition = next_local(vm);
			unsigned new_instruction_pointer = next_count(vm);

			if (as_boolean(condition))
				vm->instruction_pointer = new_instruction_pointer;

			break;
		}

		case OPCODE_JUMP_IF_FALSE: {
			value condition = next_local(vm);
			unsigned new_instruction_pointer = next_count(vm);

			if (!as_boolean(condition))
				vm->instruction_pointer = new_instruction_pointer;

			break;
		}

		case OPCODE_JUMP:
			vm->instruction_pointer = next_count(vm);
			break;

		case OPCODE_CALL: {
			value function = next_local(vm);
			unsigned arg_count = next_count(vm);
			value arguments[arg_count];

			for (unsigned i = 0; i < arg_count; i++)
				arguments[i] = next_local(vm);

			set_next_local(vm, call_value(function, arg_count, arguments, vm->env));
			break;
		}

		case OPCODE_RETURN:
			return next_local(vm);

		case OPCODE_NOT: {
			value arg = next_local(vm);
			set_next_local(vm, not_value(arg));
			break;
		}

		case OPCODE_NEGATE: {
			value arg = next_local(vm);
			set_next_local(vm, negate_value(arg));
			break;
		}

		case OPCODE_ADD: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, add_values(lhs, rhs));
			break;
		}

		case OPCODE_SUBTRACT: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, subtract_values(lhs, rhs));
			break;
		}

		case OPCODE_MULTIPLY: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, multiply_values(lhs, rhs));
			break;
		}

		case OPCODE_DIVIDE: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, divide_values(lhs, rhs));
			break;
		}

		case OPCODE_MODULO: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, modulo_values(lhs, rhs));
			break;
		}

		case OPCODE_EQUAL: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, new_boolean_value(equate_values(lhs, rhs)));
			break;
		}

		case OPCODE_NOT_EQUAL: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, new_boolean_value(!equate_values(lhs, rhs)));
			break;
		}

		case OPCODE_LESS_THAN: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) < 0));
			break;
		}

		case OPCODE_LESS_THAN_OR_EQUAL: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) <= 0));
			break;
		}

		case OPCODE_GREATER_THAN: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) > 0));
			break;
		}

		case OPCODE_GREATER_THAN_OR_EQUAL: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) >= 0));
			break;
		}

		case OPCODE_INDEX: {
			value lhs = next_local(vm);
			value rhs = next_local(vm);
			set_next_local(vm, index_value(lhs, rhs));
			break;
		}

		case OPCODE_INDEX_ASSIGN: {
			value source = next_local(vm);
			value index = next_local(vm);
			value val = next_local(vm);
			index_assign_value(source, index, val);
			set_next_local(vm, val);
			break;
		}
		}
	}

	return VNULL;
}
