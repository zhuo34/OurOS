#include "pageswap.h"

struct page_pool_struct page_pool;

void init_page_pool()
{
	page_pool.page_num = 0;
	page_pool.next_free = 1;
	for (int i = 0; i < MACHINE_PAGE_NUM; i++) {
		page_pool.entry[i] = 0;
	}
}

uint write_page_to_disk(void *vaddr, uint block_num)
{
	if (!block_num) {
		/*
		* allocate a new block for this page
		*/
		block_num = page_pool.next_free;
		uint next_next_free = page_pool.entry[page_pool.next_free - 1];
		if (next_next_free) {
			page_pool.next_free = next_next_free;
		} else {
			page_pool.next_free ++;
			page_pool.page_num ++;
		}
	}

	/*
	TODO: write to disk
	*/

	return block_num;
}

void load_page_from_disk(void *vaddr, uint block_num)
{
	
}

void delete_page_on_disk(uint block_num)
{
	page_pool.entry[block_num-1] = page_pool.next_free;
	page_pool.next_free = block_num;
}