#ifndef BUDDY_H
#define BUDDY_H

#include <ouros/buddy.h>
#include <ouros/bootmm.h>

extern bootmm_sys boot_mm;

struct page *get_page_by_idx(uint page_idx);
uint get_page_idx(struct page *page);
bool page_idx_is_in_zone(struct buddy_zone *mm, uint page_idx);
bool page_is_in_zone(struct buddy_zone *mm, struct page *page);
uint get_buddy_page_idx(uint page_idx, int bplevel);
struct page *get_buddy_page(struct page *page, int bplevel);

bool page_is_in_freelist(struct buddy_zone *mm, struct page *page);

void __free_pages(struct buddy_zone *mm, struct page *page);

void print_freelist_info();


#endif // BUDDY_H