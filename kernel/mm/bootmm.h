#ifndef BOOTMM_H
#define BOOTMM_H

#include <ouros/bootmm.h>

uint insert_bootmm_info(bootmm_sys *mm, uint start_addr, uint length, uint type);
uint set_bootmm_info(bootmm_sys *mm, uint index, uint start_addr, uint length, uint type);
uint delete_bootmm_info(bootmm_sys *mm, uint index);
uint split_bootmm_info(bootmm_sys *mm, uint index, uint split_addr);

uint find_pages(bootmm_sys *mm, uint page_cnt, uint pfn_start, uint pfn_end, uint pfn_align);

#endif // BOOTMM_H
