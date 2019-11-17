#ifndef OUROS_BOOTMM_H
#define OUROS_BOOTMM_H

#include <ouros/mm.h>

#define MAX_MMINFO_NUM	16
#define PAGE_FREE		0x00
#define PAGE_USED		0xFF

enum bootmm_info_type {
	MMINFO_TYPE_KERNEL
};

struct bootmm_info {
	uint start_addr;
	uint length;
	uint type;
};

struct bootmm_sys {
	uint phymm_size;
	uint page_num;
	uchar page_map[MACHINE_PAGE_NUM];
	uint last_alloc_end;
	uint info_cnt;
	struct bootmm_info info[MAX_MMINFO_NUM];
};

extern struct bootmm_sys boot_mm;

void init_bootmm();

void* bootmm_alloc_page(struct bootmm_sys *mm, uint size, uint type, uint addr_align);
void bootmm_free_page(struct bootmm_sys *mm, void *addrr, uint size);

#endif // OUROS_BOOTMM_H
