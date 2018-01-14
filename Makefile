simple_shell: simple_shell.c utils.c utils.h
	gcc -o simple_shell simple_shell.c utils.c utils.h

clean:
	rm -f simple_shell
