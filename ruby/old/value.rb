# you dont have to do inheritance but i thought why not
class Value
   include Comparable

   def self.new(value, *)
      return super unless self == Value # only do this specialization for `Value` itself.
      return value if value.is_a? Value

      case value
      when Integer then Int.new value
      when String then Str.new value
      when Array then Ary.new value
      when true, false then Bool.new value
      when nil then Null.new value
      else raise TypeError, "unknown kind #{value.class}"
      end
   end

   attr_reader :value

   def initialize(value)
      @value = value
   end

   def run(_env) = self

   def to_i = @value.to_i
   def to_s = @value.to_s
   def ==(rhs) = rhs.is_a?(self.class) && @value == rhs.value

   class Int < Value
      def truthy? = @value.nonzero?
      def +(rhs) = Int.new(@value + rhs.to_i)
      def -(rhs) = Int.new(@value - rhs.to_i)
      def *(rhs) = Int.new(@value * rhs.to_i)
      def /(rhs) = Int.new(@value / rhs.to_i)
      def %(rhs) = Int.new(@value % rhs.to_i)
      def <=>(rhs) = @value <=> rhs.to_i
      def -@ = Int.new(-@value)
   end

   class Str < Value
      def truthy? = !@value.zero?
      def length = @value.length
      def +(rhs) = Str.new(@value + rhs.to_s)
      def *(rhs) = Str.new(@value * rhs.to_i)
      def [](rhs) = Str.new(@value[rhs.to_i])
      def <=> = @value <=> rhs.to_i
   end

   class Bool < Value
      def truthy? = @value
      def <=> = @value <=> rhs.truthy?
   end

   class Null < Value
      def to_i = 0
      def to_s = 'null'
      def truthy? = false
   end

   class Ary < Value
      def truthy? = !@value.empty?
      undef to_i # to_i is not defined on arrays

      def length = @value.length
      def +(rhs)
         raise TypeError, "can only add arrays together (got #{rhs.class})" unless rhs.is_a? Array
         Ary.new @value + rhs.value
      end
      def [](index) = @value[index.to_i]
      def []=(index, value) @value[index.to_i] = value end
   end

   class Function < Value
      def initialize(name, args, body)
         @name, @args, @body = name, args, body
      end

      def inspect = "<function:#@name>"
      alias to_s inspect
      undef to_i

      def call(*args, env)
         unless args.length == @args.length
            raise "argument mismatch for #@name: expected #{@args.length}, got #{args.length}"
         end

         env.with_new_stackframe @args.zip(args).to_h do 
            catch :return do
               @body.run env
            end
         end
      end
   end
end
