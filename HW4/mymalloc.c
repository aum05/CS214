#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "mymalloc.h"
#include "heap_init.h"

#define ALIGNMENT  8
#define WORD      sizeof(void *) /* Word and header/footer size (bytes) */
#define TWO_WORDS      (2 * WORD)    /* Doubleword size (bytes) */
#define ADD_BLK_SIZE  (1 << 5)      /* Extend heap by this amount (bytes) */

/* Find the larger of the 2 values*/
#define GET_MAX(x, y) ((x) > (y) ? (x) : (y))

/* Fit both the size and allocated bit (0/1) in one word to save space */
#define FIT(size, alloc)  ((size) | (alloc))

/* Fetch and write at p. */
#define GET_PTR(p)       (*(uintptr_t *)(p))
#define PUT_PTR(p, val)  (*(uintptr_t *)(p) = (val))

/* Get the size and allocated status of p */
#define GET_BLK_SIZE(p)   (GET_PTR(p) & ~(TWO_WORDS - 1))
#define GET_ALLOC_STAT(p)  (GET_PTR(p) & 0x1)

/* Get ptr's header and footer */
#define HEADER(ptr)  ((void *)(ptr) - WORD)
#define FOOTER(ptr)  ((void *)(ptr) + GET_BLK_SIZE(HEADER(ptr)) - TWO_WORDS)

/* Given next and previous blocks of ptr */
#define NEXT(ptr)  ((void *)(ptr) + GET_BLK_SIZE(HEADER(ptr)))
#define PREV(ptr)  ((void *)(ptr) - GET_BLK_SIZE((void *)(ptr) - TWO_WORDS))

/* Get next and previous free blocks of ptr */
#define NEXT_FREE(ptr)  (*(char **)(ptr + WORD))
#define PREV_FREE(ptr)  (*(char **)(ptr))

/* Place next and previous pointers in the free list */
#define PUT_NEXT(ptr, pt) (NEXT_FREE(ptr) = pt)
#define PUT_PREV(ptr, pt) (PREV_FREE(ptr) = pt)

/* Helper Functions */
static void *coalesce(void *ptr);
static void *add_free_blk(size_t words);
static void *find_first_fit(size_t block_size);
static void add_block(void *ptr, size_t block_size);
static void insert_blk_list(void *ptr); 
static void delete_blk_list(void *ptr); 


/*********************** Global Variables ***********************/
static char *heap = 0;
static char *exp_free_list = 0;

int alloc_Alg = 0; /* Defines the allocation algorithm:
                    * 0 : First Fit
                    * 1 : Next Fit
                    * 2 : Best Fit
                    */

void *next_fit_ptr;

int total_req_size = 0; /* Stores the total size of requested memory through mymalloc library */
int memory_used = 0; /* The numerator for calculating the utilization */

/*********************** Helper Functions ***********************/

static void *coalesce(void *ptr){
    /* Check if previous or next block is allocated */ 
    size_t is_next_allocated = GET_ALLOC_STAT(HEADER(NEXT(ptr)));
    size_t is_prev_allocated = GET_ALLOC_STAT(FOOTER(PREV(ptr))) || PREV(ptr) == ptr;
    size_t size = GET_BLK_SIZE(HEADER(ptr));
    
    /* If only next block is free */   
    if (is_prev_allocated && !is_next_allocated) {                  
        size += GET_BLK_SIZE( HEADER(NEXT(ptr))  );
        delete_blk_list(NEXT(ptr));
        PUT_PTR(HEADER(ptr), FIT(size, 0));
        PUT_PTR(FOOTER(ptr), FIT(size, 0));
    }
    /* If only previous block is free */  
    else if (!PREV_ALLOC && is_next_allocated) {               
        size += GET_BLK_SIZE( HEADER(PREV(ptr))  );
        ptr = PREV(ptr);
        delete_blk_list(ptr);
        PUT_PTR(HEADER(ptr), FIT(size, 0));
        PUT_PTR(FOOTER(ptr), FIT(size, 0));
    }
    /* Both previous and next are free */ 
    else if (!PREV_ALLOC && !is_next_allocated) {                
        size += GET_BLK_SIZE(HEADER(PREV(ptr))) + GET_BLK_SIZE( HEADER(NEXT(ptr))  );
        delete_blk_list(PREV(ptr));
        delete_blk_list(NEXT(ptr));
        ptr = PREV(ptr);
        PUT_PTR(HEADER(ptr), FIT(size, 0));
        PUT_PTR(FOOTER(ptr), FIT(size, 0));

    }
    /* Insert the coalesced block into the free list and return ptr */
    insert_blk_list(ptr);
    return ptr;
}

static void *add_free_blk(size_t words) { 
    char *ptr;
    size_t size;

    /* Allocate words in multiple of 8 to maintain alignment */
    size = (words % 2) ? (words+1) * WORD : words * WORD;

    /* Check against minimum blocksize 16 bytes */
    if (size < 16){
        size = 16;
    }

    if ((int)(ptr = heap_sbrk(size)) == -1){ 
        return NULL;
    }

    PUT_PTR(HEADER(ptr), FIT(size, 0));         /* Initialize free block header */
    PUT_PTR(FOOTER(ptr), FIT(size, 0));         /* Initialize free block footer */
    PUT_PTR(HEADER(NEXT(ptr)), FIT(0, 1)); /* Initialize new epilogue header */
    /* Coalesce the newly added free block with next and previous blocks */
    return coalesce(ptr);
}

static void *find_first_fit(size_t block_size){
    void *ptr;
    static int last_malloced_size = 0;
    static int at_end = 0;
    if(prev_malloc == (int)block_size){
        if(at_end>30){  
            int additional_size = GET_MAX(block_size, 4 * WORD);
            ptr = add_free_blk(additional_size/4);
            return ptr;
        }
        else
            at_end++;
    }
    else
        at_end = 0;
    for (ptr = exp_free_list; GET_ALLOC_STAT(HEADER(ptr)) == 0; ptr = NEXT_FREE(ptr)){
        if (block_size <= (size_t)GET_BLK_SIZE(HEADER(ptr))) {
            prev_malloc = block_size;
            return ptr;
        }
    }
    return NULL;
}

static void *find_next_fit(size_t block_size){
    void *ptr;
    static int last_malloced_size = 0;
    static int at_end = 0;
    if(prev_malloc == (int)block_size){
        if(at_end>30){  
            int additional_size = GET_MAX(block_size, 4 * WORD);
            ptr = add_free_blk(additional_size/4);
            return ptr;
        }
        else
            at_end++;
    }
    else
        at_end = 0;

    for (ptr = next_fit_ptr; GET_ALLOC_STAT(HEADER(ptr)) == 0 && ptr == next_fit_ptr; ptr = NEXT_FREE(ptr)){
        if (block_size <= (size_t)GET_BLK_SIZE(HEADER(ptr))) {
            prev_malloc = block_size;
            next_fit_ptr = ptr;
            return ptr;
        }
    }
    return NULL;
}

static void *find_best_fit(size_t block_size){
    void *ptr;
    static int last_malloced_size = 0;
    static int at_end = 0;
    if(prev_malloc == (int)block_size){
        if(at_end>30){  
            int additional_size = GET_MAX(block_size, 4 * WORD);
            ptr = add_free_blk(additional_size/4);
            return ptr;
        }
        else
            at_end++;
    }
    else
        at_end = 0;
    for (ptr = exp_free_list; GET_ALLOC_STAT(HEADER(ptr)) == 0; ptr = NEXT_FREE(ptr)){
        if (block_size <= (size_t)GET_BLK_SIZE(HEADER(ptr))) {
            prev_malloc = block_size;
            return ptr;
        }
    }
    return NULL;
}

static void add_block(void *ptr, size_t block_size){
    size_t size = GET_BLK_SIZE(HEADER(ptr));

    if ((size - block_size) >= 4 * WORD) {
        PUT_PTR(HEADER(ptr), FIT(block_size, 1));
        PUT_PTR(FOOTER(ptr), FIT(block_size, 1));
        delete_blk_list(ptr);
        ptr = NEXT(ptr);
        PUT_PTR(HEADER(ptr), FIT(size-block_size, 0));
        PUT_PTR(FOOTER(ptr), FIT(size-block_size, 0));
        coalesce(ptr);
    }
    else {
        PUT_PTR(HEADER(ptr), FIT(size, 1));
        PUT_PTR(FOOTER(ptr), FIT(size, 1));
        delete_blk_list(ptr);
    }
}

/* Inserts the free block into the free_list */
static void insert_blk_list(void *ptr){
    PUT_NEXT(ptr, exp_free_list); 
    PUT_PREV(exp_free_list, ptr); 
    PUT_PREV(ptr, NULL); 
    exp_free_list = ptr; 
}
/* Deletes the free block into the free_list */
static void delete_blk_list(void *ptr){
    if (PREV_FREE(ptr))
        PUT_NEXT(PREV_FREE(ptr), NEXT_FREE(ptr));
    else
        exp_free_list = NEXT_FREE(ptr);
    PUT_PREV(NEXT_FREE(ptr), PREV_FREE(ptr));
}

/*********************** Implentation of mymalloc Library ***********************/

void myinit(int allocAlg){
	alloc_space();
    
    /* Initialize empty heap. */
    if ((heap = heap_sbrk(8*WORD)) == NULL) 
        printf("Error: %d\n", errno);

    PUT_PTR(heap, 0);                            /* Adjust alignment */
    PUT_PTR(heap + (1 * WORD), FIT(TWO_WORDS, 1)); /* Initialize the prologue header */ 
    PUT_PTR(heap + (2 * WORD), FIT(TWO_WORDS, 1)); /* Initialize the prologue footer */ 
    PUT_PTR(heap + (3 * WORD), FIT(0, 1));     /* Initialize the epilogue header */
    exp_free_list = heap + 2*WORD; 

    //Add a free block of minimum possible block size to initialize the heap
    if (add_free_blk(4) == NULL){ 
        printf("Error: %d\n", errno);
    }

    alloc_Alg = allocAlg;
    return;
}

void* mymalloc(size_t size){
    total_req_size += size;
    memory_used = (total_req_size > memory_used) ? total_req_size : memory_used;

    size_t block_size;      /* Block size with alignment adjustments*/
    void *ptr;
    size_t additional_size; /* If another free block required to be added */

    /* Nothing to malloc if requested size is 0 */
    if (size == 0)
        return (NULL);

    /* Adjust block size to maintain alignment */
    if (size <= TWO_WORDS)
        block_size = 2 * TWO_WORDS;
    else
        block_size = TWO_WORDS * ((size + TWO_WORDS + (TWO_WORDS - 1)) / TWO_WORDS);

    /* Find a suitable free block in the free list */
    switch (alloc_Alg){

    case 0:
        if ((ptr = find_first_fit(block_size)) != NULL) {
            add_block(ptr, block_size);
            return (ptr);
        }
        break;
    
    case 1:
        if ((ptr = find_next_fit(block_size)) != NULL) {
            add_block(ptr, block_size);
            return (ptr);
        }
        break;

    case 2:
        if ((ptr = find_first_fit(block_size)) != NULL) {
            add_block(ptr, block_size);
            return (ptr);
        }
        break;

    default:
        break;
    }

    /* If no fit found, and heapsize less than 1MB - blocksize, add free block */
    if(!((double)get_heap_size() < (1048576 - block_size)))
    	return NULL;
    	
    additional_size = GET_MAX(block_size, ADD_BLK_SIZE);
    if ((ptr = add_free_blk(additional_size / WORD)) == NULL)  
        return (NULL);
    add_block(ptr, block_size);
    return (ptr);
}

void myfree(void* ptr){
    size_t size;
    /* Nothing to free */
    if (ptr == NULL)
        return;

    size = GET_BLK_SIZE(HEADER(ptr));
    
    total_req_size -= size;

    PUT_PTR(HEADER(ptr), FIT(size, 0));
    PUT_PTR(FOOTER(ptr), FIT(size, 0));
    coalesce(ptr);
}
void* myrealloc(void* ptr, size_t size){
    if((int)size < 0) 
        return NULL; 
    else if((int)size == 0){ 
        myfree(ptr); 
        return NULL; 
    }
    else if(ptr == NULL){
        mymalloc(size);
    } 
    else if(size > 0){ 
        size_t oldsize = GET_BLK_SIZE(HEADER(ptr)); 
        size_t newsize = size + 2 * WORD;

        /* Return the ptr itself if newsize is less than oldsize */
        if(newsize <= oldsize){ 
            return ptr; 
        }
        else {
            total_req_size += (newsize - oldsize);
            memory_used = (total_req_size > memory_used) ? total_req_size : memory_used;

            size_t next_alloc = GET_ALLOC_STAT(HEADER(NEXT(ptr))); 
            size_t bsize;
            
            if(!next_alloc && ((bsize = oldsize + GET_BLK_SIZE(HEADER(NEXT(ptr))))) >= newsize){ 
                delete_blk_list(NEXT(ptr)); 
                PUT_PTR(HEADER(ptr), FIT(bsize, 1)); 
                PUT_PTR(FOOTER(ptr), FIT(bsize, 1)); 
                return ptr; 
            }
            else {  
                void *new_ptr = mymalloc(newsize);  
                add_block(new_ptr, newsize);
                memcpy(new_ptr, ptr, newsize); 
                myfree(ptr); 
                return new_ptr; 
            } 
        }
    }
    else 
        return NULL;
}

void mycleanup(){
    reset_heap();
    free_heap();
}

double utilization(){
    return ((double)memory_used / (double)get_heap_size());
}