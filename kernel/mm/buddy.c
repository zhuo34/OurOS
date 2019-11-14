#include "buddy.h"
#include <ouros/bootmm.h>
#include <ouros/utils.h>
#include <driver/vga.h>

buddy_page *all_pages;
buddy_zone buddy_mm;

void init_buddy()
{
	// allocate memory for buddy pages
	uint base_addr = (uint)bootmm_alloc_page(&boot_mm, boot_mm.page_num * sizeof(buddy_page), MMINFO_TYPE_KERNEL, 1 << PAGE_SHIFT);
	if (!base_addr) {
		
	}
	kernel_memset((void *)base_addr, 0, boot_mm.page_num * sizeof(buddy_page));
	uint virtual_addr = base_addr | 0x80000000;
	all_pages = (buddy_page *)virtual_addr;
	// init buddy pages
	for (uint i = 0; i < boot_mm.page_num; i++) {
		all_pages[i].bplevel = 0;
		all_pages[i].used_info = BUDDY_RESERVED;
		init_list_node((list_node *)(all_pages + i));
	}
	// init buddy_mm
	uint kernel_reserved_btyes = boot_mm.info[0].start_addr + boot_mm.info[0].length;
	uint kernel_reserved_pages = upper_align(kernel_reserved_btyes, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	buddy_mm.start_pfn = upper_align(kernel_reserved_pages, 1 << MAX_BUDDY_ORDER);
	buddy_mm.page_num = lower_align(boot_mm.page_num - buddy_mm.start_pfn, 1 << MAX_BUDDY_ORDER);
	buddy_mm.pages = all_pages + buddy_mm.start_pfn;	
	for (int i = 0; i < MAX_BUDDY_ORDER + 1; i++) {
		init_list((list *)&(buddy_mm.free_area[i]));
	}

	// init lock

	// init free buddy pages
	for (uint i = 0; i < buddy_mm.page_num; i++) {
		__free_pages(&buddy_mm, buddy_mm.pages + i);
	}
}

buddy_page *get_page_by_idx(uint page_idx)
{
	return all_pages + page_idx;
}

uint get_page_idx(buddy_page *page)
{
	return page - all_pages;
}

bool page_idx_is_in_zone(buddy_zone *mm, uint page_idx)
{
	return (page_idx >= mm->start_pfn) && (page_idx < mm->start_pfn + mm->page_num);
}

bool page_is_in_zone(buddy_zone *mm, buddy_page *page)
{
	return page_idx_is_in_zone(mm, get_page_idx(page));
}

uint get_buddy_page_idx(uint page_idx, int bplevel)
{
	return page_idx ^ (1 << bplevel);
}

buddy_page *get_buddy_page(buddy_page *page, int bplevel)
{
	return get_page_by_idx(get_buddy_page_idx(get_page_idx(page), bplevel));
}

bool page_is_in_freelist(buddy_zone *mm, buddy_page *page)
{
	bool ret = true;
	if (page_is_in_zone(mm, page)) {
		for (int i = 0; i < MAX_BUDDY_ORDER; i++) {
			if (!find_list_node((list *)&(mm->free_area[i]), (list_node *)page)) {
				ret = false;
				break;
			}
		}
	}
	return ret;
}

void __free_pages(buddy_zone *mm, buddy_page *page)
{
	// lock

	uint this_page_idx = get_page_idx(page);
	if (!page_idx_is_in_zone(mm, this_page_idx)) {
		kernel_printf("Free a page not in this buddy zone!");
		return;		
	}
	int bplevel = page->bplevel;
	if (page_is_in_freelist(mm, page)) {
		kernel_printf("Free a free page!");
		return;
	}
	while (bplevel < MAX_BUDDY_ORDER) {
		uint buddy_page_idx = get_buddy_page_idx(get_page_idx(page), page->bplevel);
		buddy_page *buddy_page = get_page_by_idx(buddy_page_idx);
		if (bplevel != buddy_page->bplevel || buddy_page->used_info != BUDDY_FREE) {
			break;
		}
		delete_list_node((list *)&(mm->free_area[bplevel]), (list_node *)buddy_page);
		this_page_idx &= buddy_page_idx;
		page = get_page_by_idx(this_page_idx);
		bplevel++;
	}
	page->bplevel = bplevel;
	page->used_info = BUDDY_FREE;
	append_list_node((list *)&(mm->free_area[bplevel]), (list_node *)page);

	// unlock
}

void free_pages(void *addr)
{
	__free_pages(&buddy_mm, get_page_by_idx(((uint)addr >> PAGE_SHIFT)));
}

buddy_page *__alloc_pages(buddy_zone *mm, uint bplevel)
{
	buddy_page *ret = nullptr;
	if (bplevel > MAX_BUDDY_ORDER) {
		return ret;
	}
	int free_bplevel = -1;
	// lock

	for (int i = bplevel; i <= MAX_BUDDY_ORDER; i++) {
		if (!is_empty((list *)&(mm->free_area[i]))) {
			free_bplevel = i;
			break;
		}
	}
	
	if (free_bplevel != -1) {
		buddy_page *free_page = (buddy_page *)pop_list_node((list *)&(mm->free_area[free_bplevel]));
		uint free_page_idx = get_page_idx(free_page);
		while (free_bplevel > bplevel) {
			free_bplevel--;
			uint page_not_used_idx = get_buddy_page_idx(free_page_idx, free_bplevel);
			buddy_page *page_not_used = get_page_by_idx(page_not_used_idx);
			page_not_used->bplevel = free_bplevel;
			page_not_used->used_info = BUDDY_FREE;
			append_list_node((list *)&(mm->free_area[free_bplevel]), (list_node *)page_not_used);
		}
		buddy_page *page_alloc = get_page_by_idx(free_page_idx);
		page_alloc->bplevel = bplevel;
		page_alloc->used_info = BUDDY_ALLOCED;
		ret = free_page_idx << PAGE_SHIFT;
	} else {
		kernel_printf("Buddy no enough memory!");
	}

	// unlock
	return ret;
}

void *alloc_pages(uint size)
{
	int bplevel = 0;
	int n = 1;
	int page_num = upper_align(size, 1 << PAGE_SHIFT);
	while (n < page_num) {
		n *= 2;
		bplevel++;
	}
	return __alloc_pages(&buddy_mm, bplevel);
}

void print_page_info(list_node *node)
{
	buddy_page *page = (buddy_page *)node;
	kernel_printf("%d\t%x\n", page->bplevel, page->used_info);
}

void print_buddy_info()
{
	kernel_printf("<=== buddy info ===>\n");

	kernel_printf("start pfn\t%d\n", buddy_mm.start_pfn);
	kernel_printf("page num\t%d\n", buddy_mm.page_num);
	kernel_printf("all pages addr\t%x\n", all_pages);

	kernel_printf("\n");
	print_freelist_info();

	kernel_printf("<=== buddy info end ===>\n");
}

void print_freelist_info()
{
	kernel_printf("===> free page info\n");
	for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
		kernel_printf("=>bplevel\t%d\n", i);
		traversal_list((list *)&(buddy_mm.free_area[i]), print_page_info);
	}
}

void test_buddy()
{
	init_bootmm();
	init_buddy();
	print_buddy_info();
}