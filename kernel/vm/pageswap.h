#ifndef PAGESWAP_H
#define PAGESWAP_H

#include <ouros/mm.h>

struct page_pool_struct {
	uint page_num;
	int far_next;
	uint next_free;
	uint entry[MACHINE_PAGE_NUM];
	void *block[MACHINE_PAGE_NUM];
};

extern struct page_pool_struct page_pool;

void init_page_pool();
uint write_page_to_disk(void *vaddr, uint block_num);
void load_page_from_disk(void *vaddr, uint block_num);
void delete_page_on_disk(uint block_num);

#endif // PAGESWAP_H