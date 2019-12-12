#include "vm.h"
#include <ouros/mm.h>
#include <ouros/utils.h>
#include <ouros/buddy.h>
#include <driver/vga.h>

uint **pgd_current;

void test_tlb_refill()
{
	kernel_printf("start test tlb refill\n");
	
	pgd_current = (uint **)kmalloc(sizeof(uint*) * 1024);
	pgd_current[32] = (uint*)kmalloc(sizeof(uint) * 1024);
	uint *pte = pgd_current[32];
	uint *test_vaddr = (uint *)0x08000000;
	void *paddr = alloc_one_page();
	struct page * pagep = get_page_by_paddr(paddr);
	pagep->virtual = test_vaddr;
	pte[0] = (((uint)paddr >> PAGE_SHIFT) << 6) | 0x1f;
	pte[1] = (((uint)paddr >> PAGE_SHIFT) << 6) | 0x1f;
	kernel_printf("paddr %x\n", paddr);
	
	// kernel_printf("%x\n", *test_vaddr);
	*test_vaddr = 0x646d;
	// *test_vaddr = 0xcafe;
	// kernel_printf("Status: %x\n", status);
	// kernel_printf("Cause: %x\n", cause);
	kernel_printf("%x\n", *test_vaddr);

	kernel_printf("paddr %x\n", paddr);
	free_pages(paddr);
}