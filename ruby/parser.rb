require_relative 'token'

module Friar
  class Parser
    def initialize(...)
      @tokenizer = Tokenizer.new(...)
      @peeked = nil
    end

    def peek
      @peeked ||= @tokenizer.next
    end

    def advance
      peek.tap { @peeked = nil }
    end

    def guard(...)
      if peek.match?(...)
        advance
      end
    end

    def location
      @tokenizer.location
    end

    def error(msg)
      location.error msg, 2
    end

    def take_delim(rhs, delim:)
      acc = []

      until guard rhs
        acc.push yield

        unless guard delim
          guard rhs or error "expected #{rhs.inspect} or #{delim.inspect}"
          break
        end
      end

      acc
    end
  end
end


__END__
    def atom
      paren || unary || array || variable || literal
    end

    def primary
      a = atom or return
      # todo: fncall and index
      fail
    end

    def expression
      p = primary or return
      assign(p) || index_assign(p) || binary_operator(p) || short_circuit_operator(p)
    end

    def statement
      local || self.return || self.if || self.while || continue || self.break || expression
    end

    def local
      guard 'local' or return
      name = identifier or error 'expected identifier after `local`'

      if guard '='
        init = expression or fail "expected initializer after `=`"
      end

      endline or error 'expected endline after `local`'
      Local.new name, init
    end

    def return
      fail
    end

    def if
      guard 'if' or return
      cond = expression or error "expected expression after `if`"
      body = block or error "expected body for `if`"

      if guard 'else'
        else_body = block or error "expected body for `else`"
      end

      If.new cond, body, else_body
    end

    def while
      guard 'while' or return
      cond = expression or error "expected expression after `while`"
      body = block or error "expected body for `while`"
      While.new cond, body
    end

    def continue
      fail
    end

    def break
      fail
    end

    def expression
      fail
    end

    def block
      fail
    end

    def declaration
      import || global || function
    end

    def import
      fail
    end

    def global
      fail
    end

    def function
      fail
    end
  end
end

p = Friar::Parser.new(<<EOS); p p.if
foo
EOS
