ifdef DEBUG
CFLAGS+=-g -fsanitize=address,undefined
endif
CFLAGS += -Wall -Wpedantic -Wextra
CFLAGS += -I.

all: main

.PHONY: clean
clean:
	-@rm base/*.o main

.PHONY: base
base: base/array.o base/ast.o base/environment.o base/function.o base/main.o base/number.o \
		base/shared.o base/string.o base/token.o base/value.o

.PHONY: bytecode
bytecode: bytecode/codeblock.c

main: base/array.o base/ast.o base/environment.o base/function.o base/main.o base/number.o \
		base/shared.o base/string.o base/token.o base/value.o bytecode/codeblock.o bytecode/compile.o \
		bytecode/bytecode.o bytecode/globals.o base/builtin_function.o
	$(CC) $(CFLAGS) -o $@ $+

*.o: *.c
