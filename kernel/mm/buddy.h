#ifndef BUDDY_H
#define BUDDY_H

#include <ouros/buddy.h>
#include <ouros/bootmm.h>

#define get_page_by_idx(page_idx) ((all_pages) + (page_idx))
#define get_page_by_phy_addr(page_addr) ( get_page_by_idx( ((uint)(page_addr)) >> PAGE_SHIFT ) )
#define get_page_idx(page) ((uint)((page) - (all_pages)))
#define get_page_phy_addr(page) ((void *)(get_page_idx(page) << PAGE_SHIFT))
#define page_idx_is_in_zone(mm, page_idx) (((page_idx) >= (mm)->start_pfn) && ((page_idx) < (mm)->start_pfn + (mm)->page_num))
#define page_is_in_zone(mm, page) (page_idx_is_in_zone(mm, get_page_idx(page)))
#define get_buddy_page_idx(page_idx, bplevel) ((page_idx) ^ (1 << (bplevel)))
#define get_buddy_page(page, bplevel) (get_page_by_idx(get_buddy_page_idx(get_page_idx(page), bplevel)))

bool page_is_in_freelist(struct buddy_zone *mm, struct page *page);

struct page *__alloc_pages(struct buddy_zone *mm, uint bplevel);
void __free_pages(struct buddy_zone *mm, struct page *page);

void print_freelist_info();


#endif // BUDDY_H