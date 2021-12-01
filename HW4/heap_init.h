void mem_init(void);               
void mem_deinit(void);
void *mem_sbrk(intptr_t add_space);
void mem_reset_brk(void); 
size_t mem_heapsize(void);
