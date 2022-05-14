module Friar
  class Value
    def self.from(val)
      case val
      when Value then val
      when Integer then Int.new val
      when String then Str.new val
      when Boolean then Str.new val
      else raise "unknown type: #{val}"
      end
    end
  end

  class Int < Value
    attr_reader :data

    def initialize(data)
      @data = data
    end
  end

  class Str < Value
    attr_reader :data

    def initialize(data)
      @data = data
    end
  end

  class Bool < Value
    attr_reader :data

    def initialize(data)
      @data = data
    end
  end

  class Null < Value
  end

  class Array < Value
    attr_reader :data

    def initialize(data)
      @data = data
    end
  end
end
