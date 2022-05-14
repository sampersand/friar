class Codeblock
  attr_reader :code, :nlocals, :constants

  def initialize(code, nlocals, constants)
    @code = code.freeze
    @nlocals = nlocals
    @constants = constants.freeze
  end
end

class VirtualMachine
  def initialize(block)
    @block = block
    @locals = Array.new block.nlocals
    @ip = 0
  end

  def next_ip
    @ip.tap { @ip += 1 }
  end

  def run
    while @ip < @block.code.length
      run_opcode @block.code[next_ip]
    end
    @locals[0]
  end

  def read
    @block.code[next_ip]
  end

  def run_opcode(op)
    case op
    when LOAD_CONSTANT
      @block.constants[read].then { @locals[read] = _1 }

    when LESS_THAN
      @locals[read].then { (_1 < @locals[read]).then { |x| @locals[read] = x } }

    when JUMP_IF_FALSE
      var = @locals[read]
      dst = read
      @ip = dst if var 

    when MOVE
      @

    else raise "unknown op: #{op}"
    end
  end
end


require_relative 'bytecode'
include Bytecode
vm = VirtualMachine.new(Codeblock.new([
  LOAD_CONSTANT, 0, 0, # 3
  LOAD_CONSTANT, 0, 1, # 6

  LOAD_CONSTANT, 1, 2, # 9
  LESS_THAN, 0, 2, 2, # 13
  JUMP_IF_FALSE, 2, 29, # 16

  ADD, 0, 1, 1, # 20
  LOAD_CONSTANT, 2, 2, # 23
  ADD, 0, 2, 0,
  JUMP, 13, # 29

  MOVE, 1, 0,
],
  3,
  [0, 10, 1]
)).run
