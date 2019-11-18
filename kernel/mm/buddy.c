#include "buddy.h"
#include <ouros/utils.h>
#include <driver/vga.h>

struct page *all_pages;
struct buddy_zone buddy_mm;

void init_buddy()
{
	// allocate memory for buddy pages
	uint base_addr = (uint)bootmm_alloc_page(&boot_mm, boot_mm.page_num * sizeof(struct page), MMINFO_TYPE_KERNEL, 1 << PAGE_SHIFT);
	if (!base_addr) {
		
	}
	kernel_memset((void *)base_addr, 0, boot_mm.page_num * sizeof(struct page));
	void *virtual_addr = get_kernel_vaddr(base_addr);
	all_pages = (struct page *)virtual_addr;
	// init buddy pages
	for (uint i = 0; i < boot_mm.page_num; i++) {
		all_pages[i].bplevel = 0;
		all_pages[i].used_info = BUDDY_RESERVED;
		INIT_LIST_HEAD(&(all_pages[i].list));
	}
	// init buddy_mm
	uint kernel_reserved_btyes = boot_mm.info[0].start_addr + boot_mm.info[0].length;
	uint kernel_reserved_pages = upper_align(kernel_reserved_btyes, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	buddy_mm.start_pfn = upper_align(kernel_reserved_pages, 1 << MAX_BUDDY_ORDER);
	buddy_mm.page_num = lower_align(boot_mm.page_num - buddy_mm.start_pfn, 1 << MAX_BUDDY_ORDER);
	buddy_mm.pages = all_pages + buddy_mm.start_pfn;	
	for (int i = 0; i < MAX_BUDDY_ORDER + 1; i++) {
		INIT_LIST_HEAD(&(buddy_mm.free_area[i].freelist));
	}

	// init lock

	// init free buddy pages
	for (uint i = 0; i < buddy_mm.page_num; i++) {
		__free_pages(&buddy_mm, buddy_mm.pages + i);
	}
}

bool page_is_in_freelist(struct buddy_zone *mm, struct page *page)
{
	bool ret = true;
	if (page_is_in_zone(mm, page)) {
		for (int i = 0; i < MAX_BUDDY_ORDER; i++) {
			if (!list_contain_node(&(page->list), &(mm->free_area[i].freelist))) {
				ret = false;
				break;
			}
		}
	}
	return ret;
}

void __free_pages(struct buddy_zone *mm, struct page *page)
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
		struct page *buddy_page = get_page_by_idx(buddy_page_idx);
		if (bplevel != buddy_page->bplevel || buddy_page->used_info != BUDDY_FREE) {
			break;
		}
		list_del_init(&(buddy_page->list));
		this_page_idx &= buddy_page_idx;
		buddy_page = get_page_by_idx(this_page_idx);
		bplevel++;
	}
	page->bplevel = bplevel;
	page->used_info = BUDDY_FREE;
	list_add_tail(&(page->list), &(mm->free_area[bplevel].freelist));

	// unlock
}

void free_pages(void *addr)
{
	__free_pages(&buddy_mm, get_page_by_phy_addr(addr));
}

struct page *__alloc_pages(struct buddy_zone *mm, uint bplevel)
{
	struct page *ret = nullptr;
	if (bplevel > MAX_BUDDY_ORDER) {
		return ret;
	}
	int free_bplevel = -1;
	// lock

	for (int i = bplevel; i <= MAX_BUDDY_ORDER; i++) {
		if (!list_empty(&(mm->free_area[i].freelist))) {
			free_bplevel = i;
			break;
		}
	}
	
	if (free_bplevel != -1) {
		struct page *free_page = list_last_entry(&(mm->free_area[free_bplevel].freelist), struct page, list);
		list_del_init(&(free_page->list));
		uint free_page_idx = get_page_idx(free_page);
		while (free_bplevel > bplevel) {
			free_bplevel--;
			uint page_not_used_idx = get_buddy_page_idx(free_page_idx, free_bplevel);
			struct page *page_not_used = get_page_by_idx(page_not_used_idx);
			page_not_used->bplevel = free_bplevel;
			page_not_used->used_info = BUDDY_FREE;
			list_add_tail(&(page_not_used->list), &(mm->free_area[free_bplevel].freelist));
		}
		struct page *page_alloc = get_page_by_idx(free_page_idx);
		page_alloc->bplevel = bplevel;
		page_alloc->used_info = BUDDY_ALLOCED;
		page_alloc->virtual = get_kernel_vaddr(get_page_phy_addr(page_alloc));
		page_alloc->cachep = nullptr;
		ret = page_alloc;
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
	struct page *pagep = __alloc_pages(&buddy_mm, bplevel);
	void *ret = nullptr;
	if (pagep) {
		ret = get_page_phy_addr(pagep);
	}
	return ret;
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
		struct page *p;
		list_for_each_entry(p, &(buddy_mm.free_area[i].freelist), list) {
			kernel_printf("%d\t%x\n", p->bplevel, p->used_info);
		}
	}
}

void test_buddy()
{
	init_bootmm();
	init_buddy();
	print_buddy_info();
}