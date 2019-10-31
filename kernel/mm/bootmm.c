#include "os/bootmm.h"

#include <arch.h>
#include <os/utils.h>
#include <os/assert.h>

bootmm_struct bootmm;
uchar bootmm_map[MACHINE_MMSIZE >> PAGE_SHIFT];

void init_bootmm()
{
	// init bootmm
	kernel_memset(&bootmm, 0, sizeof(bootmm));
	// init physical memory size and page frame number
	bootmm.phymm_size = get_phymm_size();
	bootmm.max_pfn = bootmm.phymm_size >> PAGE_SHIFT;
	// init map
	bootmm.map_start = bootmm_map;
	bootmm.map_end = bootmm.map_start + sizeof(bootmm_map);
	kernel_memset(bootmm.map_start, PAGE_FREE, bootmm.map_end - bootmm.map_start);
	// init info
	bootmm.info_cnt = 0;
	// alloc kernel memory 16M
	uint kmm_size = 16 * MB;
	uint kmm_pfn = kmm_size >> PAGE_SHIFT;
	insert_bootmm_info(&bootmm, 0, kmm_size - 1, MMINFO_TYPE_KERNEL);
	kernel_memset(bootmm.map_start, PAGE_USED, kmm_pfn);
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

uchar* bootmm_alloc_page(uint size, uint type, uint align)
{

}

uchar* find_pages(uint page_cnt, uint pfn_start, uint pfn_end, uint pfn_align)
{
	
}