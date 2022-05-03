#include "bytecode/codeblock.h"
#include "bytecode/globals.h"
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

#ifdef ENABLE_LOGGING
	LOGN("vm[% 3d] = local(%d) {", vm->instruction_pointer, count);
	if (local == VUNDEF) {
		printf("VUNDEF");
	} else  {
		dump_value(stdout, local);
	}
	puts("}");
#endif

	vm->instruction_pointer++;
	return local;
}

static void set_next_local(virtual_machine *vm, value val) {
	unsigned count = vm->block->code[vm->instruction_pointer].count;

#ifdef ENABLE_LOGGING
	LOGN("vm[% 3d] = local(%d) {", vm->instruction_pointer, count);
	if (val == VUNDEF) {
		printf("VUNDEF");
	} else  {
		dump_value(stdout, val);
	}
	puts("}");
#endif

	vm->instruction_pointer++;
	vm->locals[count] = val;
}

static void run_move(virtual_machine *vm) {
	set_next_local(vm, next_local(vm));
}

static void run_array_literal(virtual_machine *vm) {
	unsigned count = next_count(vm);
	array *ary = allocate_array(count);

	for (unsigned i = 0; i < count; i++)
		push_array(ary, next_local(vm));

	set_next_local(vm, new_array_value(ary));
}

static void run_load_constant(virtual_machine *vm) {
	value constant = vm->block->constants[next_count(vm)];

#ifdef ENABLE_LOGGING
	LOGN("constant=");
	dump_value(stdout, constant);
	puts("");
#endif

	set_next_local(vm, constant);
}

static void run_load_global_variable(virtual_machine *vm) {
	unsigned global_index = next_count(vm);

	set_next_local(vm, fetch_global_variable(vm->env->globals1, global_index));
}

static void run_store_global_variable(virtual_machine *vm) {
	unsigned global_index = next_count(vm);
	value value = next_local(vm);

	assign_global_variable(vm->env->globals1, global_index, value);
	set_next_local(vm, value);
}

static void run_jump_if_true(virtual_machine *vm) {
	value condition = next_local(vm);
	unsigned new_instruction_pointer = next_count(vm);

	if (as_boolean(condition))
		vm->instruction_pointer = new_instruction_pointer;
}

static void run_jump_if_false(virtual_machine *vm) {
	value condition = next_local(vm);
	unsigned new_instruction_pointer = next_count(vm);

	if (!as_boolean(condition))
		vm->instruction_pointer = new_instruction_pointer;
}

static void run_jump(virtual_machine *vm) {
	vm->instruction_pointer = next_count(vm);
}

static void run_call(virtual_machine *vm) {
	value function = next_local(vm);
	unsigned arg_count = next_count(vm);
	value arguments[arg_count];

	for (unsigned i = 0; i < arg_count; i++)
		arguments[i] = next_local(vm);

	set_next_local(vm, call_value(function, arg_count, arguments, vm->env));
}

static void run_return(virtual_machine *vm) {
	vm->instruction_pointer = (unsigned) -1; // set it to beyond the end.
}

static void run_not(virtual_machine *vm) {
	value arg = next_local(vm);

	set_next_local(vm, not_value(arg));
}

static void run_negate(virtual_machine *vm) {
	value arg = next_local(vm);

	set_next_local(vm, negate_value(arg));
}

static void run_add(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, add_values(lhs, rhs));
}

static void run_subtract(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, subtract_values(lhs, rhs));
}

static void run_multiply(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, multiply_values(lhs, rhs));
}

static void run_divide(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, divide_values(lhs, rhs));
}

static void run_modulo(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, modulo_values(lhs, rhs));
}

static void run_equal(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, new_boolean_value(equate_values(lhs, rhs)));
}

static void run_not_equal(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, new_boolean_value(!equate_values(lhs, rhs)));
}

static void run_less_than(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) < 0));
}

static void run_less_than_or_equal(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) <= 0));
}

static void run_greater_than(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) > 0));
}

static void run_greater_than_or_equal(virtual_machine *vm) {
	value lhs = next_local(vm);
	value rhs = next_local(vm);

	set_next_local(vm, new_boolean_value(compare_values(lhs, rhs) >= 0));
}

static void run_index(virtual_machine *vm) {
	value source = next_local(vm);
	value index = next_local(vm);

	set_next_local(vm, index_value(source, index));
}

static void run_index_assign(virtual_machine *vm) {
	value source = next_local(vm);
	value index = next_local(vm);
	value val = next_local(vm);
	index_assign_value(source, index, val);
	set_next_local(vm, val);
}

static void run_vm(virtual_machine *vm) {
	while (vm->instruction_pointer < vm->block->code_length) {
		switch (next_opcode(vm)) {
		case OPCODE_MOVE:          run_move(vm); break;
		case OPCODE_ARRAY_LITERAL: run_array_literal(vm); break;

		case OPCODE_LOAD_CONSTANT:         run_load_constant(vm); break;
		case OPCODE_LOAD_GLOBAL_VARIABLE:  run_load_global_variable(vm); break;
		case OPCODE_STORE_GLOBAL_VARIABLE: run_store_global_variable(vm); break;

		case OPCODE_JUMP_IF_TRUE:  run_jump_if_true(vm); break;
		case OPCODE_JUMP_IF_FALSE: run_jump_if_false(vm); break;
		case OPCODE_JUMP:          run_jump(vm); break;
		case OPCODE_CALL:          run_call(vm); break;
		case OPCODE_RETURN:        run_return(vm); break;

		case OPCODE_NOT:      run_not(vm); break;
		case OPCODE_NEGATE:   run_negate(vm); break;
		case OPCODE_ADD:      run_add(vm); break;
		case OPCODE_SUBTRACT: run_subtract(vm); break;
		case OPCODE_MULTIPLY: run_multiply(vm); break;
		case OPCODE_DIVIDE:   run_divide(vm); break;
		case OPCODE_MODULO:   run_modulo(vm); break;

		case OPCODE_EQUAL:                 run_equal(vm); break;
		case OPCODE_NOT_EQUAL:             run_not_equal(vm); break;
		case OPCODE_LESS_THAN:             run_less_than(vm); break;
		case OPCODE_LESS_THAN_OR_EQUAL:    run_less_than_or_equal(vm); break;
		case OPCODE_GREATER_THAN:          run_greater_than(vm); break;
		case OPCODE_GREATER_THAN_OR_EQUAL: run_greater_than_or_equal(vm); break;

		case OPCODE_INDEX:        run_index(vm); break;
		case OPCODE_INDEX_ASSIGN: run_index_assign(vm); break;
		}
	}
}

value run_codeblock(const codeblock *block, unsigned argc, const value *argv, environment *env) {
	virtual_machine vm;

	vm.block = block;
	vm.instruction_pointer = 0;
	vm.env = env;

	value locals[block->number_of_locals];
	vm.locals = locals;

	for (unsigned i = 0; i < block->number_of_locals; i++)
		vm.locals[i] = VUNDEF;

	for (unsigned i = 0; i < argc; i++)
		vm.locals[i + 1] = argv[i];

	run_vm(&vm);

	// note that this starts at `1`. This is because the return value is `locals[0]`.
	for (unsigned i = 1; i < block->number_of_locals; i++) {
		if (locals[i] != VUNDEF)
			free_value(locals[i]);
	}

	return locals[0];
}
