require_relative 'ast'
require_relative 'env'
require_relative 'parser'

def run_program(input)
  env = Environment.new
  parser = Parser.new input

  while (d = Declaration.parse parser)
    d.declare env
  end

  env.get_var('main').call(env)
end

input_file = ARGV.fetch(0, File.join(__dir__, '..', 'basic-ast-example.txt'))
run_program File.read input_file
