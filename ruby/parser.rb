require_relative 'token'
class Parser
  def initialize(stream)
    @tokenizer = Tokenizer.new stream
    @token = nil
  end

  def guard(kind)
    @token ||= @tokenizer.next or return

    if kind.is_a?(String) && @token.kind == :symbol && @token.value == kind
      @token.tap { @token = nil }.value
    elsif kind.is_a?(Symbol) && @token.kind == kind
      @token.tap { @token = nil }.value
    else
      nil
    end
  end

  class ParseError < RuntimeError; end

  def error(msg)
    raise ParseError, "parse error: #{msg}", caller(1)
  end

  def expect(kind, expected)
    guard(kind) or error "expected #{expected}"
  end

  def take_until_close(close, delim, context:)
    acc = []

    until guard close
      acc.push yield || error("expected #{close.inspect} in #{context}")

      unless guard delim
        expect close, "`#{close}` in #{context}, not #@token"
        break
      end
    end

    acc
  end
end
