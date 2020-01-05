#include "bootmm.h"
#include <ouros/utils.h>
#include <ouros/assert.h>

struct bootmm_sys boot_mm;

void init_bootmm()
{
	// init boot_mm
	kernel_memset(&boot_mm, 0, sizeof(boot_mm));
	// init physical memory size and page frame number
	boot_mm.phymm_size = get_phymm_size();
	boot_mm.page_num = boot_mm.phymm_size >> PAGE_SHIFT;
	// init page_map
	kernel_memset(boot_mm.page_map, PAGE_FREE, MACHINE_PAGE_NUM);
	// init info
	boot_mm.info_cnt = 0;
	// alloc kernel memory 16M
	uint kmm_size = 16 * MB;
	uint kmm_pfn = kmm_size >> PAGE_SHIFT;
	insert_bootmm_info(&boot_mm, 0, kmm_size, MMINFO_TYPE_KERNEL);
	kernel_memset(boot_mm.page_map, PAGE_USED, kmm_pfn);
}

/**
 * @brief Insert allocate information.
 * @param mm			bootmm system
 * @param start_addr	start address
 * @param length		allocate length
 * @param type			allocate type
 * @return insert information
 */
uint insert_bootmm_info(struct bootmm_sys *mm, uint start_addr, uint length, uint type)
{
	uint ret = 0;
	for (int i = 0; i < mm->info_cnt; i++) {
		struct bootmm_info *this_info = &mm->info[i];
		if (this_info->type != type)
			continue;
		if (this_info->start_addr + this_info->length == start_addr) {
			struct bootmm_info *next_info = &mm->info[i+1];
			if (next_info->type == type && start_addr + length == next_info->start_addr) {
				this_info->length += length + next_info->length;
				delete_bootmm_info(mm, i+1);
				ret = 1;
			} else {
				this_info->length += length;
				ret = 2;
			}
			break;
		} else if (start_addr + length == this_info->length) {
			this_info->start_addr = start_addr;
			this_info->length += length;
			ret = 3;
			break;
		}
	}
	if (!ret) {
		if (mm->info_cnt == MAX_MMINFO_NUM) {
			ret = 0;
		} else {
			set_bootmm_info(mm, mm->info_cnt++, start_addr, length, type);
			ret = 4;
		}
	}
	return ret;
}

uint set_bootmm_info(struct bootmm_sys *mm, uint index, uint start_addr, uint length, uint type)
{
	kernel_assert(index < MAX_MMINFO_NUM, "Set bootmm info error!");
	mm->info[index].start_addr = start_addr;
	mm->info[index].length = length;
	mm->info[index].type = type;
}

uint delete_bootmm_info(struct bootmm_sys *mm, uint index)
{
	if (index >= mm->info_cnt)
		return 0;
	for (uint i = index; i < mm->info_cnt; i++) {
		mm->info[i] = mm->info[i+1];
	}
	mm->info_cnt--;
	return 1;
}
/**
 * @brief Split allocated space.
 * @param mm				bootmm system
 * @param index				index of allocated space to be split
 * @param split_addr_start	split position
 * @return split information
 */
uint split_bootmm_info(struct bootmm_sys *mm, uint index, uint split_addr_start)
{
	if (index >= mm->info_cnt)
		return 0;
	if (mm->info_cnt == MAX_MMINFO_NUM)
		return 0;
	struct bootmm_info *mminfo = &mm->info[index];
	if (split_addr_start > mminfo->start_addr && split_addr_start < mminfo->start_addr + mminfo->length) {
		uint temp = mminfo->length;
		mminfo->length = split_addr_start - mminfo->start_addr;
		set_bootmm_info(mm, mm->info_cnt++, split_addr_start, temp - mminfo->length, mminfo->type);
		return 1;
	}
	return 0;
}

/**
 * @brief Allocate pages from bootmm system.
 * @param mm			bootmm system
 * @param size			allocated size
 * @param type			allocated type
 * @param addr_align	align
 * @return	physical address
 */
void* bootmm_alloc_page(struct bootmm_sys *mm, uint size, uint type, uint addr_align)
{
	uint page_cnt = upper_align(size, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	uint pfn_align = addr_align >> PAGE_SHIFT;
	uint res = 0;
	
	res = find_pages(mm, page_cnt, mm->last_alloc_end + 1, mm->page_num - 1, pfn_align);
	if (!res)
		res = find_pages(mm, page_cnt, 0, mm->last_alloc_end, pfn_align);
	if (res) {
		uint insert_info = insert_bootmm_info(mm, res << PAGE_SHIFT, size, type);
		if (insert_info > 0) {
			mm->last_alloc_end = res + page_cnt - 1;
			kernel_memset(mm->page_map + res, PAGE_USED, page_cnt);
		} else {
			res = 0;
		}
	}
	return (void *)(res << PAGE_SHIFT);
}

/**
 * @brief [unused] Free pages allocated by bootmm system.
 * @param mm		bootmm system
 * @param addr		address of pages to be free
 * @param size		size of pages to be free
 */
void bootmm_free_page(struct bootmm_sys *mm, void *addr, uint size)
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
	for (uint i = 0; i < mm->info_cnt; i++) {
		if ((uint)addr < mm->info[i].start_addr || (uint)addr >= mm->info[i].start_addr + mm->info[i].length) {
			continue;
		}
		uint rest_size = mm->info[i].length - ((uint)addr - mm->info[i].start_addr);
		if (rest_size < size) {
			kernel_printf("Bootmm free too much memory!");
			return;
		} else {
			uint split_info_1 = split_bootmm_info(mm, i, (uint)addr);
			if (split_info_1) {
				uint split_info_2 = split_bootmm_info(mm, mm->info_cnt-1, (uint)addr + size);
				if (split_info_2) {
					delete_bootmm_info(mm, mm->info_cnt-2);
				} else {
					delete_bootmm_info(mm, mm->info_cnt-1);
				}
			} else {
				split_bootmm_info(mm, i, (uint)addr + size);
				delete_bootmm_info(mm, i);
			}
			kernel_memset(mm->page_map + pfn_start, PAGE_FREE, pfn_cnt);
		}
	}
}

/**
 * @brief Find if bootmm has free pages cover given page number. 
 * @param mm		bootmm system
 * @param page_cnt	page number
 * @param pfn_start	search start from this
 * @param pfn_start	search end to this
 * @param pfn_align	required align
 * @return find reult
 */
uint find_pages(struct bootmm_sys *mm, uint page_cnt, uint pfn_start, uint pfn_end, uint pfn_align)
{
	if (pfn_align == 0)
		pfn_align = 1;
	pfn_start = upper_align(pfn_start, pfn_align);
	pfn_end = min(mm->page_num - 1, pfn_end);
	
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
