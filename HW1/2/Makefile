all : grep

grep :
	gcc -g -std=gnu17 -Wall -Wvla -fsanitize=address grep.c -o grep

clean :
	rm -f grep