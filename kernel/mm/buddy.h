#ifndef BUDDY_H
#define BUDDY_H

#include <ouros/buddy.h>
#include <ouros/bootmm.h>

extern bootmm_sys boot_mm;

buddy_page *get_page_by_idx(uint page_idx);
uint get_page_idx(buddy_page *page);
bool page_idx_is_in_zone(buddy_zone *mm, uint page_idx);
bool page_is_in_zone(buddy_zone *mm, buddy_page *page);
uint get_buddy_page_idx(uint page_idx, int bplevel);
buddy_page *get_buddy_page(buddy_page *page, int bplevel);

bool page_is_in_freelist(buddy_zone *mm, buddy_page *page);

void __free_pages(buddy_zone *mm, buddy_page *page);

void print_page_info(list_node *node);
void print_freelist_info();


#endif // BUDDY_H