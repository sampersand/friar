# typed: strict
require 'sorbet-runtime'

module Friar
  class Value
    extend T::Sig
    # def self.from(val)
    #   case val
    #   when Value then val
    #   when Integer then Int.new val
    #   when String then Str.new val
    #   when Boolean then Str.new val
    #   else raise "unknown type: #{val}"
    #   end
    # end
  end

  class Int < Value
    sig{ returns(Integer) }
    attr_reader :data

    sig{ params(data: Integer).void }
    def initialize(data)
      @data = data
    end
  end

  class Str < Value
    sig{ returns(String) }
    attr_reader :data

    sig{ params(data: String).void }
    def initialize(data)
      @data = data
    end
  end

  class Bool < Value
    sig{ returns(T::Boolean) }
    attr_reader :data

    sig{ params(data: T::Boolean).void }
    def initialize(data)
      @data = data
    end
  end

  class Null < Value
  end

  class Array < Value
    sig{ returns(T::Array[Value]) }
    attr_reader :data

    sig{ params(data: T::Array[Value]).void }
    def initialize(data)
      @data = data
    end
  end
end
