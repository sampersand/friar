ifdef DEBUG
	CFLAGS+=-g -fsanitize=address,undefined
else
	CFLAGS+=-O3 -flto -DNDEBUG
endif
CFLAGS += -Wall -Wpedantic -Wextra

all: main

.PHONY: clean
clean:
	-@rm src/*.o main

main: src/array.o src/ast.o src/environment.o src/function.o src/main.o src/number.o \
		src/shared.o src/string.o src/token.o src/value.o src/codeblock.o src/compile.o \
		src/bytecode.o src/globals.o src/builtin_function.o
	$(CC) $(CFLAGS) -o $@ $+

*.o: *.c
