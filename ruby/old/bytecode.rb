module Bytecode
  MOVE = 0
  ARRAY_LITERAL = 1

  LOAD_CONSTANT = 2
  LOAD_GLOBAL_VARIABLE = 3
  STORE_GLOBAL_VARIABLE = 4

  JUMP = 5
  JUMP_IF_TRUE = 6
  JUMP_IF_FALSE = 7
  CALL = 8
  RETURN = 9

  NOT = 10
  NEGATE = 11
  ADD = 12
  SUBTRACT = 13
  MULTIPLY = 14
  DIVIDE = 15
  MODULO = 16
  EQUAL = 17
  NOT_EQUAL = 18
  LESS_THAN = 19
  LESS_THAN_OR_EQUAL = 20
  GREATER_THAN = 21
  GREATER_THAN_OR_EQUAL = 22
  INDEX = 23
  INDEX_ASSIGN = 24
end

class Compiler
  def initialize
    @code = []
  end
end

class Vm
  end




