# frozen_string_literal: true
require_relative 'value'

module Friar
  class ParseError < RuntimeError
    attr_reader :location

    def initialize(location, message)
      super "#{location}: #{message}"
      @location = location
    end
  end

  class Token
    attr_reader :value
    attr_reader :kind
    attr_reader :location

    def initialize(value, kind, location)
      @value = value
      @kind = kind
      @location = location
    end

    def inspect = "Token(#{@value.inspect}, #@kind)"

    alias to_s inspect

    def match?(clause)
      case clause
      when String then @kind == :symbol && @value == clause
      when Symbol then @kind == clause
      end
    end
  end

  class SourceLocation
    attr_reader :file, :line, :column

    def initialize(file, line, column)
      @file = file
      @line = line
      @column = column
    end

    def to_s
      "#@file at #@line:#@column"
    end

    def error(msg, levels_up=1)
      raise ParseError, "#{self}: #{msg}", caller(levels_up)
    end
  end

  class Tokenizer
    KEYWORDS = %w[
      global import
      function local
      return if else while continue break
    ].freeze

    def initialize(source, file: '-e')
      @source = source.chars
      @file = file 
      @line = 1
      @column = 1
    end

    def location
      SourceLocation.new @file, @line, @column
    end

    def next
      start = location

      case peek
      when nil then nil # at eof, `peek` returns `nil`

      # strip whitespace
      when /\s/
        take_while_regex /\s/
        self.next

      # strip comments
      when '#'
        take_while { _1 != "\n" }
        self.next

      # Number literals
      when /\d/
        int = take_while_regex(/\d/).to_i

        if peek.match? /\w/
          location.error "invalid suffix after integer literal"
        end

        Token.new Int.new(int), :value, start

      # Single quoted strings are just their literal contents.
      when "'"
        text = take_while { _1 != "'" }

        unless advance == "'"
          start.error "unterminated single quote encountered"
        end

        Token.new Str.new(text), :value, start

      # Double quoted strings interpolate their contents somewhat
      when '"'
        text_start = location
        text = take_while { _1 != '"' }

        unless advance == '"'
          text_start.error "unterminated double quote encountered"
        end

        # todo interpolation

        Token.new Str.new(text), :string, start

      # identifiers
      when /\w/
        word = take_while_regex /\w/
        case word
        when 'true'    then Token.new Bool.new(true), :value, start
        when 'false'   then Token.new Bool.new(false), :value, start
        when 'null'    then Token.new Null.new, :value, start
        when *KEYWORDS then Token.new word, :symbol, start
        else                Token.new word, :identifier, start
        end

      when /[-+*\/%<>=!&|,;\(\)\[\]\{\}]/
        symbol = advance

        if %w[! = < >].include?(symbol) && peek == '='
          symbol.concat advance
        end

        Token.new symbol, :symbol, start

      else
        start.error "unknown token start: #{@source[0].inspect}"
      end
    end

    private

    def peek
      @source.first
    end

    def advance
      chr = @source.shift or location.error "`.advance` when at EOF"

      if chr == "\n"
        @line += 1
        @column = 1
      else
        @column += 1
      end
      chr
    end

    def take_while_regex(regex)
      take_while { regex.match? _1 }
    end

    def take_while(&block)
      acc = ""
      acc.concat advance while peek&.then(&block) # `&.then` so eof doesn't call block.
      acc.empty? ? nil : acc
    end
  end
end
