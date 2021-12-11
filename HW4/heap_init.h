void alloc_space(void);               
void free_heap(void);
void *heap_sbrk(intptr_t add_space);
void reset_heap(void); 
size_t get_heap_size(void);