class Environment
  BUILTINS = {
    'print' => proc { puts _1 },
    'push' => proc { _1.push _2 },
    'pop' => proc { _1.pop },
    'length' => proc { Value.new _1.length },
  }

  def initialize
    @globals = BUILTINS.dup
    @stackframes = []
  end

  def with_new_stackframe(args)
    @stackframes.push args
    yield
  ensure
    @stackframes.pop
  end

  def declare_global(name, value)
    @globals[name] = value
  end

  def assign_var(name, value)
    if @globals.include? name
      @globals[name] = value
    else
      @stackframes.last[name] = value
    end
  end

  def get_var(name)
    @globals[name] || @stackframes.last[name]
  end
end
