#include "buddy.h"
#include <ouros/utils.h>

buddy_page *buddy_pages;
buddy_zone buddy_mm;

void init_buddy()
{
	uint base_addr = (uint)bootmm_alloc_page(&bootmm, bootmm.max_pfn * sizeof(buddy_page), MMINFO_TYPE_KERNEL, 1 << PAGE_SHIFT);
	if (!base_addr) {
		
	}
	uint virtual_addr = base_addr | 0x80000000;
	buddy_pages = (buddy_page *)virtual_addr;

	uint kernel_reserved_btyes = bootmm.info[0].addr_start + bootmm.info[0].length;
	uint kernel_reserved_pages = upper_align(kernel_reserved_btyes, 1 << PAGE_SIZE) >> PAGE_SIZE;
	buddy_mm.start_pfn = upper_align(kernel_reserved_pages, 1 << MAX_BUDDY_ORDER);
	buddy_mm.max_pfn = lower_align(bootmm.max_pfn - buddy_mm.start_pfn, 1 << MAX_BUDDY_ORDER);
	

}

