all : grep

grep :
	gcc -g -Wall -Wvla -fsanitize=address grep.c -o grep

clean :
	rm -f grep