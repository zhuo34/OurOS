#include "buddy.h"
#include <ouros/bootmm.h>
#include <ouros/utils.h>

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
	uint kernel_reserved_btyes = boot_mm.info[1].start_addr + boot_mm.info[0].length;
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

void __free_pages(buddy_zone *mm, buddy_page *page)
{

}
