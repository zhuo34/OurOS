#ifndef OUROS_BUDDY_H
#define OUROS_BUDDY_H

#include <ouros/type.h>
#include <ouros/list.h>

struct buddy_page_struct {
	list_node list;
	bool used;
	uint bplevel;
};
typedef struct buddy_page_struct buddy_page;

struct buddy_free_area_struct {
	list freelist;
};
typedef struct buddy_free_area_struct buddy_free_area;

#define MAX_BUDDY_ORDER 4

struct buddy_zone_struct {
	uint start_pfn;
	uint max_pfn;
	buddy_page *pages;
	buddy_free_area free_area[MAX_BUDDY_ORDER + 1];	// 0, 1, ..., MAX_BUDDY_ORDER
};
typedef struct buddy_zone_struct buddy_zone;

void init_buddy();

#endif // OUROS_BUDDY_H
