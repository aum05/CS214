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
    enum {MALLOC, FREE, REALLOC} type;
    int index;                          /* Used by free to identify the allocation operation */
    int size;                           /* Requested memory allocation */
} operation;

typedef struct {
    unsigned num_ops;       /* Number of operations */
    operation *ops;         /* Array that stores info for each operation */
    char **blocks;          /* Array of pointers from mymalloc & myrealloc */
} test_runs;

static test_runs *init_run(){
    test_runs *run;
    if ((run = (test_runs *) malloc(sizeof(test_runs))) == NULL) {
	    printf("malloc failed");
        EXIT_FAILURE;
    }

    run->num_ops = 500000;

    if ((run->ops = (operation *)malloc(run->num_ops * sizeof(operation))) == NULL){
	    printf("malloc failed");
        EXIT_FAILURE;
    }
    if ((run->blocks = (char **)malloc(run->num_ops * sizeof(char *))) == NULL){
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
    while(i < run->num_ops){
        if (op_num==1 || op_num==2){
            size = (rand() % 256) + 1;
        }
        switch (op_num) {
        case 1:
            run->ops[i].type = MALLOC;
            run->ops[i].index = index;
            run->ops[i].size = size;
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
            
            run->ops[i].type = REALLOC;
            run->ops[i].index = r_index;
            run->ops[i].size = size;
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

            run->ops[i].type = FREE;
            run->ops[i].index = f_index;
            i++;
            break;

        default:
            break;
        }
        op_num = (rand() % 3) + 1;
        
    }
    
    return run;
}

void free_run(test_runs *run){
    free(run->ops);
    free(run->blocks);      
    free(run);
}

int main(int argc, char** argv){
    unsigned i, index, size, newsize;
    char *p, *newp, *oldp, *block;

    /************************** First Fit Algorithm **************************/
    test_runs *first = init_run();
    

    myinit(0);

    struct timeval start_time_first;
    struct timeval end_time_first;
    double time_elapsed_first;

    double tp_first; /* Stores the throughput of the first fit allocation algorithm*/
    double util_first; /* Stores the utilization of the first fit allocation algorithm*/
    
    gettimeofday(&start_time_first, NULL);

    for (i = 0;  i < first->num_ops;  i++){
    	printf("type: %u\t index: %d\t size: %d\n",first->ops[i].type, first->ops[i].index, first->ops[i].size);
    	printf("heapsize: %f\n", (double)get_heap_size());
        switch (first->ops[i].type) {
		
        case MALLOC: /* mymalloc */
            index = first->ops[i].index;
            size = first->ops[i].size;
            if ((p = mymalloc(size)) == NULL)
		        printf("mm_malloc error in eval_mm_speed");
                EXIT_FAILURE;
            first->blocks[index] = p;
            break;

        case FREE: /* myfree */
            index = first->ops[i].index;
            block = first->blocks[index];
            myfree(block);
            break;

        case REALLOC: /* myrealloc */
	        index = first->ops[i].index;
            newsize = first->ops[i].size;
	        oldp = first->blocks[index];
            if ((newp = myrealloc(oldp,newsize)) == NULL)
		        printf("mm_realloc error in eval_mm_speed");
                EXIT_FAILURE;
            first->blocks[index] = newp;
            break;

        default:
            break;

        }
    }

    gettimeofday(&end_time_first, NULL);

    time_elapsed_first = (end_time_first.tv_sec - start_time_first.tv_sec);
    tp_first = first->num_ops/time_elapsed_first;

    util_first = utilization();

    printf("First fit throughput: %f ops/sec\n", tp_first);
    printf("First fit utilization: %f\n", util_first);
    
    /*printf("num ops: %d\n",first->num_ops);
    for(int i=0; i<first->num_ops; i++){
        printf("type: %u\t index: %d\t size: %d\n",first->ops[i].type, first->ops[i].index, first->ops[i].size);
    }*/
    
    mycleanup();
    
    free_run(first);

    /************************** Next Fit Algorithm **************************/
    test_runs *next = init_run();

    myinit(1);

    struct timeval start_time_next;
    struct timeval end_time_next;
    double time_elapsed_next;

    double tp_next; /* Stores the throughput of the next fit allocation algorithm*/
    double util_next; /* Stores the utilization of the next fit allocation algorithm*/
    
    gettimeofday(&start_time_next, NULL);

    for (i = 0;  i < next->num_ops;  i++){
    	printf("type: %u\t index: %d\t size: %d\n",next->ops[i].type, next->ops[i].index, next->ops[i].size);
    	printf("heapsize: %f\n", (double)get_heap_size());
        switch (next->ops[i].type) {
		
        case MALLOC: /* mymalloc */
            index = next->ops[i].index;
            size = next->ops[i].size;
            if ((p = mymalloc(size)) == NULL)
		        printf("mm_malloc error in eval_mm_speed");
                EXIT_FAILURE;
            next->blocks[index] = p;
            break;

        case FREE: /* myfree */
            index = next->ops[i].index;
            block = next->blocks[index];
            myfree(block);
            break;

        case REALLOC: /* myrealloc */
	        index = next->ops[i].index;
            newsize = next->ops[i].size;
	        oldp = next->blocks[index];
            if ((newp = myrealloc(oldp,newsize)) == NULL)
		        printf("mm_realloc error in eval_mm_speed");
                EXIT_FAILURE;
            next->blocks[index] = newp;
            break;

        default:
            break;

        }
    }

    gettimeofday(&end_time_next, NULL);

    time_elapsed_next = (end_time_next.tv_sec - start_time_next.tv_sec);
    tp_next = next->num_ops/time_elapsed_next;

    util_next = utilization();

    printf("Next fit throughput: %f ops/sec\n", tp_next);
    printf("Next fit utilization: %f\n", util_next);
    
    /*printf("num ops: %d\n",next->num_ops);
    for(int i=0; i<next->num_ops; i++){
        printf("type: %u\t index: %d\t size: %d\n",next->ops[i].type, next->ops[i].index, next->ops[i].size);
    }*/
    
    mycleanup();
    free_run(next);

    /************************** Best Fit Algorithm **************************/
    test_runs *best = init_run();

    myinit(2);

    struct timeval start_time_best;
    struct timeval end_time_best;
    double time_elapsed_best;

    double tp_best; /* Stores the throughput of the best fit allocation algorithm*/
    double util_best; /* Stores the utilization of the best fit allocation algorithm*/
    
    gettimeofday(&start_time_best, NULL);

    for (i = 0;  i < best->num_ops;  i++){
    	printf("type: %u\t index: %d\t size: %d\n",best->ops[i].type, best->ops[i].index, best->ops[i].size);
    	printf("heapsize: %f\n", (double)get_heap_size());
        switch (best->ops[i].type) {
		
        case MALLOC: /* mymalloc */
            index = best->ops[i].index;
            size = best->ops[i].size;
            if ((p = mymalloc(size)) == NULL)
		        printf("mm_malloc error in eval_mm_speed");
                EXIT_FAILURE;
            best->blocks[index] = p;
            break;

        case FREE: /* myfree */
            index = best->ops[i].index;
            block = best->blocks[index];
            myfree(block);
            break;

        case REALLOC: /* myrealloc */
	        index = best->ops[i].index;
            newsize = best->ops[i].size;
	        oldp = best->blocks[index];
            if ((newp = myrealloc(oldp,newsize)) == NULL)
		        printf("mm_realloc error in eval_mm_speed");
                EXIT_FAILURE;
            best->blocks[index] = newp;
            break;

        default:
            break;

        }
    }

    gettimeofday(&end_time_best, NULL);

    time_elapsed_best = (end_time_best.tv_sec - start_time_best.tv_sec);
    tp_best = best->num_ops/time_elapsed_best;

    util_best = utilization();

    printf("Best fit throughput: %f ops/sec\n", tp_best);
    printf("Best fit utilization: %f\n", util_best);
    
    /*printf("num ops: %d\n",best->num_ops);
    for(int i=0; i<best->num_ops; i++){
        printf("type: %u\t index: %d\t size: %d\n",best->ops[i].type, best->ops[i].index, best->ops[i].size);
    }*/
    
    mycleanup();
    free_run(best);

    return EXIT_SUCCESS;
}