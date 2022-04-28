ifdef DEBUG
CFLAGS+=-g -fsanitize=address,undefined
endif

all: main

.PHONY: clean
clean:
	-@rm *.o main

main: main.o token.o ast.o value.o run.o environment.o shared.o
	$(CC) $(CFLAGS) -o $@ $+

*.o: *.c
