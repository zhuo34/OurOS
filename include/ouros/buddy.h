#ifndef OUROS_BUDDY_H
#define OUROS_BUDDY_H

#include <ouros/type.h>
#include <ouros/list.h>

struct page_struct {
	list_node *list;
	bool used;
	uint bplevel;
};
typedef struct page_struct page;

struct buddy_zone_struct {
	page *pages;
	
};
typedef struct buddy_zone_struct buddy_zone;

void init_buddy();

#endif // OUROS_BUDDY_H
