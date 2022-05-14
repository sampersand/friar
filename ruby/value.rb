module Friar
  class Value
    def self.from(val)
      case val
      when Value then val
      when Integer then Int.new val
      when String then Str.new val
      else raise "unknown type: #{val}"
      end
    end

  end

  class Int < Value
    def initialize(data)
      @data = data
    end
  end

  class Str < Value
    def initialize(data)
      @data = data
    end
  end

  class Bool < Value
    def initialize(data)
      @data = data
    end
  end

  class Null < Value
    def initialize
      @data = nil
    end
  end
end
