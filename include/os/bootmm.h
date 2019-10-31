#ifndef OS_BOOTMM_H
#define OS_BOOTMM_H

#include <os/type.h>

#define MAX_MMINFO_NUM	16
#define PAGE_SIZE		4096
#define PAGE_SHIFT		12
#define PAGE_FREE		0x00
#define PAGE_USED		0xFF

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
	uchar *map_start;
	uchar *map_end;
	uint last_alloc_end;
	uint info_cnt;
	struct _bootmm_info info[MAX_MMINFO_NUM];
};

typedef struct _bootmm_info bootmm_info;
typedef struct _bootmm_struct bootmm_struct;

void init_bootmm();

uchar* bootmm_alloc_page(uint size, uint type, uint align);

#endif // OS_BOOTMM_H
