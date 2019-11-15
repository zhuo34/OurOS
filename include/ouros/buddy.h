#ifndef OUROS_BUDDY_H
#define OUROS_BUDDY_H

#include <ouros/type.h>
#include <ouros/list.h>

enum buddy_page_info {
	BUDDY_FREE, BUDDY_SLAB, BUDDY_ALLOCED, BUDDY_RESERVED
};

struct page {
	struct list_head list;
	uint used_info;
	int bplevel;
};

struct buddy_free_area {
	struct list_head freelist;
	int size;
};

#define MAX_BUDDY_ORDER 4

struct buddy_zone {
	uint start_pfn;
	uint page_num;
	struct page *pages;
	struct buddy_free_area free_area[MAX_BUDDY_ORDER + 1];	// 0, 1, ..., MAX_BUDDY_ORDER
};

void init_buddy();

void free_pages(void *addr);
void *alloc_pages(uint size);

void print_buddy_info();

void test_buddy();

#endif // OUROS_BUDDY_H
