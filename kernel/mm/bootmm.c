/*
 * @Author: Zhuo Chen
 * @Github: https://github.com/dmdecz
 * @Description: 
 * @FilePath: /OurOs/kernel/mm/bootmm.c
 */
#include "bootmm.h"
#include <ouros/utils.h>
#include <ouros/assert.h>

bootmm_struct bootmm;
// uchar bootmm_map[MACHINE_MMSIZE >> PAGE_SHIFT];

void init_bootmm()
{
	// init bootmm
	kernel_memset(&bootmm, 0, sizeof(bootmm));
	// init physical memory size and page frame number
	bootmm.phymm_size = get_phymm_size();
	bootmm.max_pfn = bootmm.phymm_size >> PAGE_SHIFT;
	// init page_map
	kernel_memset(bootmm.page_map, PAGE_FREE, KERNEL_PAGE_NUM);
	// init info
	bootmm.info_cnt = 0;
	// alloc kernel memory 16M
	uint kmm_size = 16 * MB;
	uint kmm_pfn = kmm_size >> PAGE_SHIFT;
	insert_bootmm_info(&bootmm, 0, kmm_size - 1, MMINFO_TYPE_KERNEL);
	kernel_memset(bootmm.page_map, PAGE_USED, kmm_pfn);
}

uint insert_bootmm_info(bootmm_struct *mm, uint addr_start, uint length, uint type)
{
	uint ret = 0;
	for (int i = 0; i < mm->info_cnt; i++) {
		bootmm_info this_info = mm->info[i];
		if (this_info.type != type)
			continue;
		if (this_info.addr_start + this_info.length == addr_start) {
			bootmm_info next_info = mm->info[i+1];
			if (next_info.type == type && addr_start + length == next_info.addr_start) {
				this_info.length += length + next_info.length;
				delete_bootmm_info(mm, i+1);
				ret = 1;
			} else {
				this_info.length += length;
				ret = 2;
			}
			break;
		} else if (addr_start + length == this_info.length) {
			this_info.addr_start = addr_start;
			this_info.length += length;
			ret = 3;
			break;
		}
	}
	if (!ret) {
		if (mm->info_cnt == MAX_MMINFO_NUM) {
			ret = 0;
		} else {
			set_bootmm_info(mm, mm->info_cnt++, addr_start, length, type);
			ret = 4;
		}
	}
	return ret;
}

uint set_bootmm_info(bootmm_struct *mm, uint index, uint addr_start, uint length, uint type)
{
	kernel_assert(index < MAX_MMINFO_NUM, "Set bootmm info error!");
	mm->info[index].addr_start = addr_start;
	mm->info[index].length = length;
	mm->info[index].type = type;
}

uint delete_bootmm_info(bootmm_struct *mm, uint index)
{
	if (index >= mm->info_cnt)
		return 0;
	for (uint i = index; i < mm->info_cnt; i++) {
		mm->info[i] = mm->info[i+1];
	}
	mm->info_cnt--;
	return 1;
}

uint split_bootmm_info(bootmm_struct *mm, uint index, uint split_addr_start)
{
	if (index >= mm->info_cnt)
		return 0;
	if (mm->info_cnt == MAX_MMINFO_NUM)
		return 0;
	bootmm_info mminfo = mm->info[index];
	if (split_addr_start > mminfo.addr_start && split_addr_start < mminfo.addr_start + mminfo.length) {
		uint temp = mminfo.length;
		mminfo.length = split_addr_start - mminfo.addr_start;
		set_bootmm_info(mm, mm->info_cnt++, split_addr_start, temp - mminfo.length, mminfo.type);
		return 1;
	}
	return 0;
}

/**
 * @description:
 * @param
 * @return
 */
void* bootmm_alloc_page(uint size, uint type, uint addr_align)
{
	uint page_cnt = upper_align(size, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	uint pfn_align = addr_align >> PAGE_SHIFT;
	uint res = 0;
	
	res = find_pages(&bootmm, page_cnt, bootmm.last_alloc_end + 1, bootmm.max_pfn - 1, pfn_align);
	if (!res)
		res = find_pages(&bootmm, page_cnt, 0, bootmm.last_alloc_end, pfn_align);
	if (res) {
		uint insert_info = insert_bootmm_info(&bootmm, res << PAGE_SHIFT, size, type);
		if (insert_info > 0) {
			bootmm.last_alloc_end = res + page_cnt - 1;
			kernel_memset(bootmm.page_map + res, PAGE_USED, page_cnt);
		} else {
			res = 0;
		}
	}
	return (void *)(res << PAGE_SHIFT);
}

void bootmm_free_page(void *addr, uint size)
{
	if (get_low_bits((uint)addr, PAGE_SHIFT) != 0) {
		kernel_printf("Address not aligned!");
		return;
	}
	uint pfn_start = (uint)addr >> PAGE_SHIFT;
	if (pfn_start < KERNEL_PAGE_NUM) {
		kernel_printf("Bootmm free kernel memroy!");
		return;
	}
	uint pfn_cnt = size >> PAGE_SHIFT;
	if (!pfn_cnt) {
		kernel_printf("Bootmm free less a page!");
		return;
	}
	for (uint i = 0; i < bootmm.info_cnt; i++) {
		if ((uint)addr < bootmm.info[i].addr_start || (uint)addr >= bootmm.info[i].addr_start + bootmm.info[i].length) {
			continue;
		}
		uint rest_size = bootmm.info[i].length - ((uint)addr - bootmm.info[i].addr_start);
		if (rest_size < size) {
			kernel_printf("Bootmm free too much memory!");
			return;
		} else {
			uint split_info_1 = split_bootmm_info(&bootmm, i, (uint)addr);
			if (split_info_1) {
				uint split_info_2 = split_bootmm_info(&bootmm, bootmm.info_cnt-1, (uint)addr + size);
				if (split_info_2) {
					delete_bootmm_info(&bootmm, bootmm.info_cnt-2);
				} else {
					delete_bootmm_info(&bootmm, bootmm.info_cnt-1);
				}
			} else {
				split_bootmm_info(&bootmm, i, (uint)addr + size);
				delete_bootmm_info(&bootmm, i);
			}
			kernel_memset(bootmm.page_map + pfn_start, PAGE_FREE, pfn_cnt);
		}
	}
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
uint find_pages(bootmm_struct *mm, uint page_cnt, uint pfn_start, uint pfn_end, uint pfn_align)
{
	if (pfn_align == 0)
		pfn_align = 1;
	pfn_start = upper_align(pfn_start, pfn_align);
	pfn_end = min(mm->max_pfn - 1, pfn_end);
	
	for (uint i = pfn_start; i <= pfn_end;) {
		if (mm->page_map[i] != PAGE_USED) {
			uint cnt = page_cnt - 1;
			uint j = i + 1;
			while (cnt) {
				if (j > pfn_end)
					return 0;
				if (mm->page_map[j++] == PAGE_USED)
					break;
				cnt--;
			}
			if (cnt) {
				i = upper_align(j, pfn_align);
			} else {
				return i;
			}
		} else {
			i = upper_align(i+1, pfn_align);
		}
	}
	return 0;
}
