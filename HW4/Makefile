CC = gcc
CFLAGS = -m32 -Wall -Wextra -O2 -g

OBJS = memperf.o mymalloc.o heap_init.o

memperf: $(OBJS)
	$(CC) $(CFLAGS) -o memperf $(OBJS)

memperf.o: memperf.c mymalloc.h heap_init.h
heap_init.o: heap_init.c heap_init.h
mymalloc.o: mymalloc.c mymalloc.h heap_init.h

clean:
	rm -f *~ *.o memperf