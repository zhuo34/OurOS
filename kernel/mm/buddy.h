#ifndef BUDDY_H
#define BUDDY_H

#include <ouros/buddy.h>
#include <ouros/bootmm.h>

#define pgn_is_in_zone(mm, pgn) (((pgn) >= (mm)->start_pfn) && ((pgn) < (mm)->start_pfn + (mm)->page_num))
#define page_is_in_zone(mm, page) (pgn_is_in_zone(mm, get_pgn(page)))
#define get_buddy_pgn(pgn, bplevel) ((pgn) ^ (1 << (bplevel)))
#define get_buddy_page(pagep, bplevel) (get_page_by_pgn(get_buddy_pgn(get_pgn(pagep), bplevel)))

bool page_is_in_freelist(struct buddy_zone *mm, struct page *page);

struct page *__alloc_pages(struct buddy_zone *mm, uint bplevel);
void __free_pages(struct buddy_zone *mm, struct page *page);

void print_freelist_info();


#endif // BUDDY_H