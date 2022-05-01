ifdef DEBUG
CFLAGS+=-g -fsanitize=address,undefined
endif
CFLAGS += -Wall -Wpedantic -Wextra
CFLAGS += -I.

all: main

.PHONY: clean
clean:
	-@rm *.o main


main: base/array.o base/ast.o base/environment.o base/function.o base/main.o base/number.o \
		base/shared.o base/string.o base/token.o base/value.o
	$(CC) $(CFLAGS) -o $@ $+

*.o: *.c
