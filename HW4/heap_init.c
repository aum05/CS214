#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "heap_init.h"

#define HEAPSIZE   1048576  /* 1 MB */

static char *heap_start;    /* First byte of heap */
static char *heap_brk;      /* last byte of heap */
static char *max_heap;      /* Max heap address */ 

void alloc_space(void)
{
    /* allocate 1MB memory to heap*/
    if ((heap_start = (char *)malloc(HEAPSIZE)) == NULL) {
        fprintf(stderr, "malloc error\n");
        exit(1);
    }

    max_heap = heap_start + HEAPSIZE;  /* Max heap address */
    heap_brk = heap_start;             /* Empty heap */
}

void free_heap(void)
{
    free(heap_start);
}

void reset_heap()
{
    heap_brk = heap_start;
}

void *heap_sbrk(intptr_t add_space) 
{
    char *original_brk = heap_brk;

    if ((add_space < 0) || ((heap_brk + add_space) > max_heap)) {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: Ran out of memory...\n");
        return (void *)-1;
    }
    heap_brk += add_space;
    return (void *)original_brk;
}

size_t get_heap_size() 
{
    return (size_t)(heap_brk - heap_start);
}