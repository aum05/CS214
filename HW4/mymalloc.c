#include "mymalloc.h"

static char *mem_start_brk;  /* points to first byte of heap */
static char *mem_brk;        /* points to last byte of heap */
static char *mem_max_addr;   /* largest legal heap address */ 

void *mem_sbrk(intptr_t incr) 
{
    char *old_brk = mem_brk;

    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
	errno = ENOMEM;
	fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
	return (void *)-1;
    }
    mem_brk += incr;
    return (void *)old_brk;
}

void myinit(int allocAlg){
   /* allocate the storage we will use to model the available VM */
    if ((mem_start_brk = (char *)malloc(MAX_HEAP)) == NULL) {
        fprintf(stderr, "mem_init_vm: malloc error\n");
        exit(1);
    }

    mem_max_addr = mem_start_brk + MAX_HEAP;  /* max legal heap address */
    mem_brk = mem_start_brk;                  /* heap is empty initially */

        /* Create the initial empty heap. */
    if ((heap_listp = mem_sbrk(8*WSIZE)) == NULL) 
        return -1;

    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */ 
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    free_list_start = heap_listp + 2*WSIZE; 

    /* Extend the empty heap with a free block of minimum possible block size */
    if (extend_heap(4) == NULL){ 
        return -1;
    }
    return 0;
}
void* mymalloc(size_t size){
    
}
void myfree(void* ptr){
   
}
void* myrealloc(void* ptr, size_t size){

}

void mycleanup(){

}
double utilization(){

}