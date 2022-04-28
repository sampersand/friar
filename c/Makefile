all: main

.PHONY: clean
clean:
	-@rm *.o main

main: main.o token.o ast.o value.o run.o env.o

*.o: *.c
