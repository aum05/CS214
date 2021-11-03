CC=gcc
DEPS = headers.h
OBJ = built_ins.o basic_setup.o shell.o run_command.o pinfo.o parse.o io_redirection.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
		gcc -o $@ $^ $(CFLAGS)
clean :
		rm -f shell.o
		rm -f basic_setup.o
		rm -f built_ins.o
		rm -f run_command.o
		rm -f pinfo.o
		rm -f parse.o
		rm -f io_redirection.o
		rm -f shell