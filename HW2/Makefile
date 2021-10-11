all : ls find tree

ls :
	gcc -g -std=gnu17 -Wall -Wvla -fsanitize=address ls.c -o ls

find :
	gcc -g -std=gnu17 -Wall -Wvla -fsanitize=address find.c -o find

tree :
	gcc -g -std=gnu17 -Wall -Wvla -fsanitize=address tree.c -o tree

clean :
	rm -f ls
	rm -f find
	rm -f tree