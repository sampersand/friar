require_relative 'value'
module Declaration
  def self.parse(parser)
    Global.parse(parser) || Function.parse(parser)
  end

  class Global
    def initialize(name)
      @name = name
    end

    def self.parse(parser)
      parser.guard 'global' or return
      name = parser.expect :identifier, 'an identifier after `global`'
      new name
    end

    def declare(env)
      env.declare_global @name, Value.new(nil)
    end
  end

  class Function
    def initialize(name, args, body)
      @name, @args, @body = name, args, body
    end

    def self.parse(parser)
      parser.guard 'function' or return
      name = parser.expect :identifier, 'function name'

      parser.expect '(', 'function arguments start'
      args = parser.take_until_close ')', ',', context: 'function arguments' do
        parser.expect :identifier, 'argument name'
      end

      body = Block.parse(parser) or parser.error("expected body after function defn for `#{name}`")
      new name, args, body
    end

    def declare(env)
      env.declare_global @name, Value::Function.new(@name, @args, @body)
    end
  end
end

class Block
  def initialize(statements)
    @statements = statements
  end

  # block := '{' {[<statement>] ';'} '}'
  def self.parse(parser)
    parser.guard '{' or return

    statements = []
    until parser.guard '}'
      statements.push statement(parser) || parser.error("unable to parse statement")
    end

    new statements
  end

  def run(env)
    @statements.each do |statement|
      statement.run env
    end
  end

  # statement := <return-stmt> | <if-stmt> | <while-stmt> | <continue-stmt> | <break-stmt> ;
  def self.statement(parser)
    stmt = If.parse(parser) || While.parse(parser) and return stmt
    stmt = Return.parse(parser) ||
      Continue.parse(parser) ||
      Break.parse(parser) ||
      Expression.parse(parser) or return

    parser.expect ';', '`;` after statement'
    return stmt
  end

  class Return
    def initialize(value)
      @value = value
    end

    # return-stmt := 'return' [<expression>] ;
    def self.parse(parser)
      parser.guard 'return' or return
      value = Expression.parse(parser) || Literal.new(nil)
      new value
    end

    def run(env)
      throw :return, @value.run(env)
    end
  end

  class If
    def initialize(condition, body, else_body)
      @condition, @body, @else_body = condition, body, else_body
    end

    # if-stmt := 'if' <expression> <block> ['else' <block>];
    def self.parse(parser)
      parser.guard 'if' or return
      condition = Expression.parse(parser) or parser.error 'missing if condition'
      body = Block.parse(parser) or parser.error 'missing if body'

      # no `else if`, it's `else { if ... }`
      if parser.guard 'else'
        else_body = Block.parse(parser) or parser.error 'missing else body'
      end

      new condition, body, else_body
    end

    def run(env)
      if @condition.run(env).truthy?
        @body.run env
      elsif @else_body
        @else_body.run env
      end
    end
  end

  class While
    def initialize(condition, body)
      @condition, @body = condition, body
    end

    # while-stmt := 'while' <expression> <block> ;
    def self.parse(parser)
      parser.guard 'while' or return
      condition = Expression.parse(parser) or parser.error 'missing while condition'
      body = Block.parse(parser) or parser.error 'missing while body'

      new condition, body
    end

    def run(env)
      # I'm effectively using exceptions as control flow here, except ruby has a nice `catch/throw`
      # thing which doesn't generate stack traces, and is thus much more efficient than exceptions.
      catch :break do
        while @condition.run(env).truthy?
          catch :continue do
            @body.run env
          end
        end
      end
    end
  end

  class Continue
    # continue-stmt := 'continue' ;
    def self.parse(parser)
      parser.guard 'continue' and new
    end

    def run(_env)
      throw :continue
    end
  end

  class Break
    # break-stmt := 'break' ;
    def self.parse(parser)
      parser.guard 'break' and new
    end

    def run(_env)
      throw :break
    end
  end
end

module Expression
  # expression
  #  := <simple-assignment>
  #   | <index-assignment>
  #   | <binary-op>
  #   | <primary>
  #   ;
  def self.parse(parser)
    primary = Primary.parse(parser) or return

    case
    when sa = SimpleAssignment.parse(primary, parser) then sa
    when ia = IndexAssignment.parse(primary, parser) then ia
    when bo = BinaryOperator.parse(primary, parser) then bo
    else primary
    end
  end

  class BinaryOperator
    OPERATORS = %w[+ - * / % < > <= >= == != & |].freeze

    def initialize(op, lhs, rhs)
      @op, @lhs, @rhs = op, lhs, rhs
    end

    # binary-op := <primary> BINARY_OP <expression>;
    def self.parse(lhs, parser)
      # I'm going to just ignore order of operations for my sanity
      op = OPERATORS.find { parser.guard _1 } or return
      rhs = Expression.parse(parser) or parser.error "expected rhs for op #{op}"
      new op, lhs, rhs
    end

    def run(env)
      Value.new @lhs.run(env).public_send(@op, @rhs.run(env))
    end
  end

  class SimpleAssignment
    def initialize(var, value)
      @var, @value = var, value
    end

    # simple-assignment := IDENTIFIER '=' <expression> ;
    def self.parse(variable, parser)
      return unless variable.is_a? Primary::Variable

      parser.guard '=' or return
      value = Expression.parse(parser) or parser.error "expected rhs for simple assignment"

      new variable.var, value
    end

    def run(env)
      env.assign_var @var, @value.run(env)
    end
  end

  class IndexAssignment
    def initialize(src, index, value)
      @src, @index, @value = src, index, value
    end

    # index-assignment := <indexing> '=' <expression> ;
    def self.parse(indexing, parser)
      return unless indexing.is_a? Primary::Indexing

      parser.guard '=' or return
      value = Expression.parse(parser) or parser.error "expected rhs for index assignment"

      new indexing.src, indexing.index, value
    end

    def run(env)
      # we use `.[]=` to ensure the operands are evaluated in the expected order,
      # as `a[b]=c` evaluates `c` before `b` in some ruby versions.
      @src.run(env).[]=(@index.run(env), @value.run(env))
    end
  end
end

module Primary
  # primary
  #  := <paren-expression>
  #   | <indexing>
  #   | <function-call>
  #   | <unary-op>
  #   | <array-literal>
  #   | <variable>
  #   | <literal>
  #   ;
  def self.parse(parser)
    primary = paren_expression(parser) ||
      UnaryOperator.parse(parser) ||
      ArrayLiteral.parse(parser) ||
      Variable.parse(parser) ||
      Literal.parse(parser) or return

    begin
      if prim_ = Indexing.parse(primary, parser)
        primary = prim_
      elsif prim_ = FunctionCall.parse(primary, parser) 
        primary = prim_
      end
    end while p

    primary
  end

  # paren-expr := '(' <expression> ')'
  def self.paren_expression(parser) # we don't need a separate class for this
    parser.guard '(' or return
    expr = Expression.parse(parser) or parser.error 'expected expression after `(`'
    parser.expect ')', '`)` after paren expression'
    expr
  end

  class Indexing
    attr_reader :src, :index

    def initialize(src, index)
      @src, @index = src, index
    end

    # indexing := <primary> '[' <expression> ']' ;
    def self.parse(primary, parser)
      parser.guard '[' or return
      index = Expression.parse(parser) or parser.error "expected expression for array index"
      parser.expect ']', 'array index'

      new primary, index
    end

    def run(env)
      @src.run(env)[@index.run(env)]
    end
  end

  class FunctionCall
    def initialize(func, args)
      @func, @args = func, args
    end

    # function-call := <primary> '(' {<expression> ','} [<expression>] ')' ;
    def self.parse(primary, parser)
      parser.guard '(' or return

      args = parser.take_until_close ')', ',', context: 'function call' do
        Expression.parse parser
      end

      new primary, args
    end

    def run(env)
      @func.run(env).call(*@args.map { _1.run env }, env)
    end
  end

  class UnaryOperator
    OPERATORS = %w[+ - !].freeze

    def initialize(op, rhs)
      @op, @rhs = op, rhs
    end

    # unary-op := UNARY_OP <primary> ;
    def self.parse(parser)
      op = OPERATORS.find { parser.guard _1 } or return
      rhs = Primary.parse(parser) or parser.error "expected rhs for op #{op}"
      new op, rhs
    end

    def run(env)
      rhs = @rhs.run env

      case @op
      when '+' then rhs # no-op
      when '-' then -rhs
      when '!' then Value.new !rhs.truthy? # overwriting `!` in ruby is especially bad practice.
      else fail "unknown op: #@op"
      end
    end
  end

  class ArrayLiteral
    def initialize(ary)
      @ary = ary
    end

    # array-literal := '[' {<expression> ','} [<expression>] ']' ;
    def self.parse(parser)
      parser.guard '[' or return

      ary = parser.take_until_close ']', ',', context: 'array literal' do
        Expression.parse parser
      end

      new ary
    end

    def run(env)
      Value.new @ary.map { _1.run env }
    end
  end

  class Variable
    attr_reader :var

    def initialize(var)
      @var = var
    end

    # variable := IDENTIFIER ;
    def self.parse(parser)
      var = parser.guard(:identifier) and return new var
    end

    def run(env)
      env.get_var @var or raise "unknown variable name #@var"
    end
  end


  class Literal
    def initialize(value)
      @value = value
    end

    # literal := STRING | INTEGER | 'true' | 'false' | 'null' ;
    def self.parse(parser)
      new Value.new case
                    when integer = parser.guard(:integer) then integer
                    when string = parser.guard(:string) then string
                    when literal = parser.guard(:literal) then literal
                    else return
                    end
    end

    def run(_env)
      @value
    end
  end
end
