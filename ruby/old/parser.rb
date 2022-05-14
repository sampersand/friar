require_relative 'token'
class Parser
  def initialize(contents, file: nil)
    @tokenizer = Tokenizer.new contents, file: file
    @peeked_token = nil
  end

  def guard(kind)
    @token ||= @tokenizer.next or return

    if kind.is_a?(String) && @token.kind == :symbol && @token.value == kind
      @token.tap { @token = nil }.value
    elsif kind.is_a?(Symbol) && @token.kind == kind
      @token.tap { @token = nil }.value
    else
      nil
    end
  end

  class ParseError < RuntimeError; end

  def error(msg)
    raise ParseError, "parse error: #{msg}", caller(1)
  end

  def expect(kind, expected)
    guard(kind) or error "expected #{expected}"
  end

  def take_until_close(close, delim, context:)
    acc = []

    until guard close
      acc.push yield || error("expected #{close.inspect} in #{context}")

      unless guard delim
        expect close, "`#{close}` in #{context}, not #@token"
        break
      end
    end

    acc
  end


  # def primary
  #   paren_expression
  #     || indexing
  #     || function_call
  #     || unary_op
  #     || array_literal
  #     || variable
  #     || literal
  # end



# module Primary
#   # primary
#   #  := <paren-expression>
#   #   | <indexing>
#   #   | <function-call>
#   #   | <unary-op>
#   #   | <array-literal>
#   #   | <variable>
#   #   | <literal>
#   #   ;
#   def self.parse(parser)
#     primary = paren_expression(parser) ||
#       UnaryOperator.parse(parser) ||
#       ArrayLiteral.parse(parser) ||
#       Variable.parse(parser) ||
#       Literal.parse(parser) or return

#     begin
#       if prim_ = Indexing.parse(primary, parser)
#         primary = prim_
#       elsif prim_ = FunctionCall.parse(primary, parser) 
#         primary = prim_
#       end
#     end while p

#     primary
#   end

#   # paren-expr := '(' <expression> ')'
#   def self.paren_expression(parser) # we don't need a separate class for this
#     parser.guard '(' or return
#     expr = Expression.parse(parser) or parser.error 'expected expression after `(`'
#     parser.expect ')', '`)` after paren expression'
#     expr
#   end

#   class Indexing
#     attr_reader :src, :index

#     def initialize(src, index)
#       @src, @index = src, index
#     end

#     # indexing := <primary> '[' <expression> ']' ;
#     def self.parse(primary, parser)
#       parser.guard '[' or return
#       index = Expression.parse(parser) or parser.error "expected expression for array index"
#       parser.expect ']', 'array index'

#       new primary, index
#     end

#     def run(env)
#       @src.run(env)[@index.run(env)]
#     end
#   end

#   class FunctionCall
#     def initialize(func, args)
#       @func, @args = func, args
#     end

#     # function-call := <primary> '(' {<expression> ','} [<expression>] ')' ;
#     def self.parse(primary, parser)
#       parser.guard '(' or return

#       args = parser.take_until_close ')', ',', context: 'function call' do
#         Expression.parse parser
#       end

#       new primary, args
#     end

#     def run(env)
#       @func.run(env).call(*@args.map { _1.run env }, env)
#     end
#   end

#   class UnaryOperator
#     OPERATORS = %w[+ - !].freeze

#     def initialize(op, rhs)
#       @op, @rhs = op, rhs
#     end

#     # unary-op := UNARY_OP <primary> ;
#     def self.parse(parser)
#       op = OPERATORS.find { parser.guard _1 } or return
#       rhs = Primary.parse(parser) or parser.error "expected rhs for op #{op}"
#       new op, rhs
#     end

#     def run(env)
#       rhs = @rhs.run env

#       case @op
#       when '+' then rhs # no-op
#       when '-' then -rhs
#       when '!' then Value.new !rhs.truthy? # overwriting `!` in ruby is especially bad practice.
#       else fail "unknown op: #@op"
#       end
#     end
#   end

#   class ArrayLiteral
#     def initialize(ary)
#       @ary = ary
#     end

#     # array-literal := '[' {<expression> ','} [<expression>] ']' ;
#     def self.parse(parser)
#       parser.guard '[' or return

#       ary = parser.take_until_close ']', ',', context: 'array literal' do
#         Expression.parse parser
#       end

#       new ary
#     end

#     def run(env)
#       Value.new @ary.map { _1.run env }
#     end
#   end

#   class Variable
#     attr_reader :var

#     def initialize(var)
#       @var = var
#     end

#     # variable := IDENTIFIER ;
#     def self.parse(parser)
#       var = parser.guard(:identifier) and return new var
#     end

#     def run(env)
#       env.get_var @var or raise "unknown variable name #@var"
#     end
#   end

  def literal
    Value.new 


#   class Literal
#     def initialize(value)
#       @value = value
#     end

#     # literal := STRING | INTEGER | 'true' | 'false' | 'null' ;
#     def self.parse(parser)
#       new Value.new case
#                     when integer = parser.guard(:integer) then integer
#                     when string = parser.guard(:string) then string
#                     when literal = parser.guard(:literal) then literal
#                     else return
#                     end
#     end

#     def run(_env)
#       @value
#     end
#   end
# end
end
