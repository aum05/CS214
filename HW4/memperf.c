#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>

#include "heap_init.h"
#include "mymalloc.h"

typedef struct {
    enum {MALLOC, FREE, REALLOC} type; /* type of request */
    int index;                        /* index for free() to use later */
    int size;                         /* byte size of alloc/realloc request */
} operation;

typedef struct {
    unsigned num_ops;         /* number of distinct requests */
    operation *ops;      /* array of requests */
    char **blocks;       /* array of ptrs returned by malloc/realloc... */
    size_t *block_sizes; /* ... and a corresponding array of payload sizes */
} test_runs;

static test_runs *init_run(){
    test_runs *first_run;
    if ((first_run = (test_runs *) malloc(sizeof(test_runs))) == NULL) {
	    printf("malloc failed");
        EXIT_FAILURE;
    }

    first_run->num_ops = 500000;

    if ((first_run->ops = (operation *)malloc(first_run->num_ops * sizeof(operation))) == NULL){
	    printf("malloc failed");
        EXIT_FAILURE;
    }
    if ((first_run->blocks = (char **)malloc(first_run->num_ops * sizeof(char *))) == NULL){
	    printf("malloc failed");
        EXIT_FAILURE;
    }
    if ((first_run->block_sizes = (size_t *)malloc(first_run->num_ops * sizeof(size_t))) == NULL){
	    printf("malloc failed");
        EXIT_FAILURE;
    }

    int op_num = 1; /* if 1: mymalloc
                     * if 2: myrealloc
                     * if 3: myfree
                     */

    int index = 0; /* Stores the index of the mymalloc, so myfree can find the pointer */
    int r_index = 0;
    int f_index = 0;
    int size = 0;
    srand(time(0));
    
    int is_used[500000] = { 0 }; /* flags */
    int i = 0;
    while(i < first_run->num_ops){
        if (op_num==1 || op_num==2){
            size = (rand() % 256) + 1;
        }
        switch (op_num) {
        case 1:
            first_run->ops[i].type = MALLOC;
            first_run->ops[i].index = index;
            first_run->ops[i].size = size;
            index++;
            i++;
            break;
        
        case 2:
            r_index = (rand() % (index+1));
            if (is_used[r_index] == 1 && index <= 1)
                break;
            else
                r_index = rand() % index;
               
            if(is_used[r_index] == 1){
            	break;
            }
            
            first_run->ops[i].type = REALLOC;
            first_run->ops[i].index = r_index;
            first_run->ops[i].size = size;
            i++;
            break;

        case 3:
            f_index = (rand() % (index+1));
            if (is_used[f_index] == 1 && index <= 1)
                break;
            else
                f_index = rand() % index;
            
            if(is_used[f_index] == 1){
            	break;
            }
            //assert(is_used[f_index] == 0);
            is_used[f_index] = 1;

            first_run->ops[i].type = FREE;
            first_run->ops[i].index = f_index;
            i++;
            break;

        default:
            break;
        }
        op_num = (rand() % 3) + 1;
        
    }
    
    return first_run;
}

int main(int argc, char** argv){
    /* First Fit Algorithm*/
    test_runs *run = init_run();
    unsigned i, index, size, newsize;
    char *p, *newp, *oldp, *block;

    myinit(0);

    struct timeval start_time;
    struct timeval end_time;
    double time_elapsed;

    double tp_first; /* Stores the throughput of the first fit allocation algorithm*/
    double util_first; /* Stores the utilization of the first fit allocation algorithm*/
    
    gettimeofday(&start_time, NULL);
    printf("first seconds : %ld\n", start_time.tv_sec);

    for (i = 0;  i < run->num_ops;  i++){
    	printf("type: %u\t index: %d\t size: %d\n",run->ops[i].type, run->ops[i].index, run->ops[i].size);
    	printf("heapsize: %f\n", (double)mem_heapsize());
        switch (run->ops[i].type) {
		
        case MALLOC: /* mymalloc */
            index = run->ops[i].index;
            size = run->ops[i].size;
            if ((p = mymalloc(size)) == NULL)
		        printf("mm_malloc error in eval_mm_speed");
                EXIT_FAILURE;
            run->blocks[index] = p;
            break;

        case FREE: /* myfree */
            index = run->ops[i].index;
            block = run->blocks[index];
            myfree(block);
            break;

        case REALLOC: /* myrealloc */
	        index = run->ops[i].index;
            newsize = run->ops[i].size;
	        oldp = run->blocks[index];
            if ((newp = myrealloc(oldp,newsize)) == NULL)
		        printf("mm_realloc error in eval_mm_speed");
                EXIT_FAILURE;
            run->blocks[index] = newp;
            break;

        default:
            break;

        }
    }

    gettimeofday(&end_time, NULL);
    printf("seconds : %ld\n", end_time.tv_sec);

    time_elapsed = (end_time.tv_sec - start_time.tv_sec);
    tp_first = time_elapsed/run->num_ops;

    util_first = utilization();

    printf("First fit throughput: %f ops/sec\n", tp_first);
    printf("First fit utilization: %f\n", util_first);
    
    /*printf("num ops: %d\n",run->num_ops);
    for(int i=0; i<run->num_ops; i++){
        printf("type: %u\t index: %d\t size: %d\n",run->ops[i].type, run->ops[i].index, run->ops[i].size);
    }*/
    
    mycleanup();
    
    return EXIT_SUCCESS;
}