class Token
   attr_reader :value, :kind

   def initialize(value, kind)
      @value = value
      @kind = kind
   end

   def inspect = "Token(#@value, #@kind)"
   alias to_s inspect
end

class Tokenizer
   def initialize(input)
      @source = input
   end

   KEYWORDS = %w[global function return if else while continue break].freeze

   def next
      # strip comments and leading whitespace
      while @source.slice!(/\A\s+/) || @source.slice!(/\A#.*/)
         # do nothing
      end

      case
      when @source.empty? then nil
      when @source.slice!(/\A\d+\b/) then Token.new $&.to_i, :integer
      when @source.slice!(/\A(?:"([^"]*)"|'([^']*)')/) then Token.new $+, :string
      when @source.slice!(/\A(true|false)/) then Token.new $&=='true', :literal
      when @source.slice!(/\Anull/) then Token.new nil, :literal
      when @source.slice!(/\A\w+\b/) then Token.new $&, KEYWORDS.include?($&) ? :symbol : :identifier
      when @source.slice!(/\A([!=<>]=|[-+*\/%<>=!&|,;\(\)\[\]\{\}])/) then Token.new $&, :symbol
      else raise "unknown token start: #{@source[0].inspect}"
      end
   end
end
