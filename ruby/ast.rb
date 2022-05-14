# typed: false
module Friar
  module Ast
    module Atom
      def self.parse(parser)
        ArrayLiteral.parse(parser) ||
          Literal.parse(parser) ||
          Variable.parse(parser) ||
          Unary.parse(parser)
      end

      class ArrayLiteral
        def self.parse(parser)
          return unless parser.guard '['
          # contents = parser.surround '[', ']', ',', context: "array literal" do
          #   Expression.parse parser
          # end or return
          new parser.take_delim(']', delim: ',') {
            Expression.parse parser or parser.error "expected expression within array literal"
          } 
        end

        def initialize(contents)
          @contents = contents
        end
      end

      class Literal
        def self.parse(parser)
          value = parser.guard(:value) or return

          new value
        end

        def initialize(value)
          @value = value
        end
      end

      class Variable
        def self.parse(parser)
          ident = parser.guard(:identifier) or return

          new ident.value
        end

        def initialize(name)
          @name = name
        end
      end

      class Unary
        UNARY_OPS = %w[+ - !]

        def self.parse(parser)
          op = UNARY_OPS.find { parser.guard _1 } or return
          rhs = Primary.parse(parser) or parser.error "expected rhs to unary #{unary}"

          new op, rhs
        end

        def initialize(op, rhs)
          @op = op
          @rhs = rhs
        end
      end
    end
  end
end
