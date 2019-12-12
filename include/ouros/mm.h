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

#define get_kernel_vaddr(paddr) ((void *)(((uint)(paddr)) | KERNEL_ENTRY))
#define get_kernel_paddr(virtual_addr) ((void *)(((uint)(virtual_addr)) & (~KERNEL_ENTRY)))

extern void init_bootmm();

extern void init_buddy();
extern void free_pages(void *addr);
extern void *alloc_pages(uint size);
extern void *alloc_one_page();

extern void init_slab();
extern void *kmalloc(uint size);
extern void kfree(void *objp);

#endif // OUROS_MM_H