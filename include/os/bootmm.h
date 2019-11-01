#ifndef OS_BOOTMM_H
#define OS_BOOTMM_H

#include <arch.h>
#include <os/type.h>

#define MAX_MMINFO_NUM	16
#define PAGE_SIZE		4096
#define PAGE_SHIFT		12
#define PAGE_FREE		0x00
#define PAGE_USED		0xFF

#define KERNEL_PAGE_NUM	(MACHINE_MMSIZE >> PAGE_SHIFT)

enum bootmm_info_type {
	MMINFO_TYPE_KERNEL
};

struct _bootmm_info {
	uint addr_start;
	uint length;
	uint type;
};

struct _bootmm_struct {
	uint phymm_size;
	uint max_pfn;
	uchar page_map[KERNEL_PAGE_NUM];
	uint last_alloc_end;
	uint info_cnt;
	struct _bootmm_info info[MAX_MMINFO_NUM];
};

typedef struct _bootmm_info bootmm_info;
typedef struct _bootmm_struct bootmm_struct;

void init_bootmm();

void* bootmm_alloc_page(uint size, uint type, uint addr_align);
void bootmm_free_page(void *addrr, uint size);

#endif // OS_BOOTMM_H
