#include "vm.h"
#include "pageswap.h"

#include <tlb.h>
#include <ouros/mm.h>
#include <ouros/utils.h>
#include <ouros/buddy.h>
#include <driver/vga.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

struct mm_struct *mm_current;
// int debug = 0;

void test_tlb_refill()
{
	init_page_pool();
	kernel_printf("start test page fault\n");
	
	mm_current = create_mm_struct();	
	kernel_printf("mm_current created.\n");
	while (1);
	// uint *test_vaddr = (uint *)0x08000000;
	// uint *test_vaddr1 = (uint *)0x08004000;
	// uint *test_vaddr2 = (uint *)0x08008000;
	// uint *test_vaddr3 = (uint *)0x0800C000;
	int *test_vaddrs[4];

	for (int i = 0; i < 4; i++) {
		test_vaddrs[i] = (int *)(0x08000000 + 0x00004000 * i);
		kernel_printf("access %x\n", test_vaddrs[i]);
		int a = 5555 + i;
		
		*(test_vaddrs[i]) = a;
		kernel_printf("test %d\n", *(test_vaddrs[i]));
		// while (1);
		// if (i == 2) {
		// 	while (1);
		// }
	}
	while (1);
	// debug = 1;
	kernel_printf("access %x\n", test_vaddrs[0]);
	kernel_printf("test %d\n", *(test_vaddrs[0]));
	kernel_printf("end test page fault\n");
	while (1);
}

struct mm_struct *create_mm_struct()
{
	kernel_printf("alloc mm\n");
	struct mm_struct *mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct));
	kernel_printf("mm: %x\n", mm);
	while (1);
	kernel_printf("alloc pgd\n");
	mm->pgd = create_pt();
	kernel_printf("pgd: %x\n", mm->pgd);
	kernel_printf("pgd: %x\n", mm->pgd);
	while (1);
	mm->pf_num = 0;
	INIT_LIST_HEAD(&mm->fifo);
	return mm;
}

pgd_t *create_pt()
{
	pgd_t *ret = (pgd_t *)kmalloc(sizeof(pgd_t) * PGD_NUM);
	for (int i = 0; i < PGD_NUM; i++) {
		ret[i] = (pte_t *)kmalloc(sizeof(pte_t) * PTE_NUM);
	}
	return ret;
}

void init_pt(pgd_t *pgd)
{
	for (int i = 0; i < PGD_NUM; i++) {
		for (int j = 0; j < PTE_NUM; j++) {
			pgd[i][j].tlb_entry.value = 0;
			pgd[i][j].info = 0;
		}
	}
}

pte_t *get_pte(pgd_t *pgd, void *vaddr)
{
	uint addr = (uint) vaddr;
	uint pgd_idx = addr >> 22;
	uint pte_idx = (addr << 10) >> 22;
	return &(pgd[pgd_idx][pte_idx]);
}

bool pte_is_valid(pte_t *pte)
{
	return pte->tlb_entry.reg.V;
}

u32 get_pte_tlb_entry(pte_t *pte)
{
	return pte->tlb_entry.value;
}

void set_pte_tlb_entry(pte_t *pte, u32 value)
{
	pte->tlb_entry.value = value;
}

int find_vma(struct mm_struct *mm, void *vaddr)
{
	return 1;
}

void do_pagefault(void *vaddr)
{
	kernel_printf("do pagefault\n");
	// if (debug) while (1);
	int vma = find_vma(mm_current, vaddr);
	if (vma) {
		pte_t *pte = get_pte(mm_current->pgd, vaddr);
		/**
		 * If this virtual address is in vma
		 * 
		 * Need to reallocate a physical page frame for this virtual page.
		 * Swapping page frame is needed when there is no free physical page frame
		 * from BUDDY system.
		 */
		void *paddr = nullptr;
		if (mm_current->pf_num < MAX_PFN_PER_PROCESS) {
			paddr = alloc_one_page();
			if (paddr) {
				/**
				 * If there is free page frame,
				 * 1. set virtual address of this page
				 * 2. add this page to FIFO list
				 * 3. pfn_num of this process increament
				 */
				struct page *pagep = get_page_by_paddr(paddr);
				kernel_printf("alloc phy addr: %x\n", paddr);
				// while (1);
				pagep->virtual = vaddr;
				list_add_tail(&pagep->list, &mm_current->fifo);
				mm_current->pf_num ++;
			} else {
				/**
				 * If there is no free page frame
				 * Swap page frame
				 */
				paddr = swap_one_page(mm_current, vaddr);
			}
		} else {
			/**
			 * Swap page frame from this process
			 */
			paddr = swap_one_page(mm_current, vaddr);
		}
		uint lo = (((uint)paddr >> PAGE_SHIFT) << 6) | 0x1f;
		set_pte_tlb_entry(pte, lo);
	} else {
		/**
		 * 2. this virtual address is not in vma
		 * TODO: kill the process
		 */
	}
}

void *swap_one_page(struct mm_struct *mm, void *vaddr)
{
	struct page *pagep = list_first_entry(&mm->fifo, struct page, list);
	void *vaddr_out = pagep->virtual;
	kernel_printf("swap out vaddr_out %x\n", vaddr_out);
	pte_t *pte = get_pte(mm->pgd, vaddr_out);
	void *paddr = (void *) (pte->tlb_entry.reg.PFN << PAGE_SHIFT);
	kernel_printf("swap out paddr %x\n", paddr);

	kernel_printf("new block num: %d\n", write_page_to_disk(vaddr_out, pte->info));
	// if (debug) while (1);
	load_page_from_disk(vaddr_out, get_pte(mm->pgd, vaddr)->info);
	/**
	 * 1. tlb reset
	 * 2. delete the page swapped out from the fifo
	 * ! Attention:
	 * In MIPS architecture, nesting tlb miss will return to the origin EPC. 
	 * So put 'delete' at last in case:
	 * another tlb miss is triggered in 'write_page_to_disk' before,
	 * due to the victim page virtual address is not in tlb.
	*/
	pte->tlb_entry.reg.V = 0;
	tlb_reset(vaddr_out);
	// while (1);

	pagep->virtual = vaddr;
	list_del(&pagep->list);
	return paddr;
}

u32 get_entry_hi(void *vaddr)
{
	union EntryHi entry_hi;
	entry_hi.value = 0;
	entry_hi.reg.VPN2 = (uint)vaddr >> 13;
	/**
	 * TODO: set entry_hi 's ASID field
	 */
	return entry_hi.value;
}

#pragma GCC pop_options