#ifndef OUROS_MM_H
#define OUROS_MM_H

#include <arch.h>
#include <ouros/type.h>
#include <ouros/utils.h>

#define PAGE_SIZE		4096
#define PAGE_SHIFT		12

#define MACHINE_PAGE_NUM	(MACHINE_MMSIZE >> PAGE_SHIFT)
#define KERNEL_MMSIZE		(MB << 4)
#define KERNEL_PAGE_NUM		(KERNEL_MMSIZE >> PAGE_SHIFT)

#define get_kernel_vaddr(phy_addr) ((void *)(((uint)(phy_addr)) | KERNEL_ENTRY))
#define get_kernel_paddr(virtual_addr) ((void *)(((uint)(virtual_addr)) & (~KERNEL_ENTRY)))

#endif // OUROS_MM_H