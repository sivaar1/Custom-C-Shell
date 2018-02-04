all: simple_shell

simple_shell: simple_shell.o utils.o
	gcc -o simple_shell simple_shell.o utils.o

simple_shell.o: simple_shell.c utils.c
	gcc -c simple_shell.c utils.c

utils.o: utils.c
	gcc -c utils.c

clean:
	rm -rf *.o simple_shell
