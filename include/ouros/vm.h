#ifndef OUROS_VM_H
#define OUROS_VM_H

#include <ouros/type.h>
#include <regs.h>
#include <linux/list.h>
#include <ouros/mm.h>

#define MAX_PFN_PER_PROCESS MACHINE_PAGE_NUM

extern int page_fault_debug;

/**
 * ****************************
 * +  Virtual Memory in MIPS  +
 * ****************************
 * MIPS virtual memory space can be divided into 4 parts:
 *                   ++++++++++++++++++++++
 *                   |    kernel (1 G)    |
 *        kseg2      |   mapped, cached   |
 *    0xC000_0000 => ++++++++++++++++++++++
 *                   |   kernel (512 M)   |
 *        kseg1      | unmapped, uncached |
 *    0xA000_0000 => ++++++++++++++++++++++
 *                   |   kernel (512 M)   |
 *        kseg0      |  unmapped, cached  |
 *    0x8000_0000 => ++++++++++++++++++++++
 *                   |     user (2 G)     |
 *         useg      |   mapped, cached   |
 *    0x0000_0000 => ++++++++++++++++++++++
 * 
 * Unmapped means not using MMU, uncached means not using cache.
 * The former is easy to convert virtual address to physical address:
 * virtual => physical: reset highest 3 bits.
 * 
 * In two unmapped segment, kseg1 does not use cache, for I/O, DMA, etc.
 * kseg0 is managed by kmalloc/kfree.
 * 
 * kseg2 is just like useg, using MMU, but is only used in kernel,
 * managed by vmalloc (Unimplemented).
 * 
 * useg is for user space.
 * 
 * ****************
 * +  Page Table  +
 * ****************
 * 32 bit virtual address => 4 GB virtual space
 * => 1 M pages => 8 MB page table (8 B per entry)
 * 
 * 2-level page table, with 10 bits pgd index, 10 bits pte index, 12 page offset
 * virtual address:
 *        +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *        |  pgd 10 bits  |  pgd 10 bits  |  page offset 10 bits  |
 *        +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#define PGD_NUM (1 << 10)
#define PTE_NUM (1 << 10)

struct __pte_t {
	union EntryLo tlb_entry;
	u32 info;
};
typedef struct __pte_t pte_t;
typedef struct __pte_t* pgd_t;

extern struct mm_struct *mm_current;

struct vm_area_struct {

};

struct mm_struct {
	int count;
	pgd_t *pgd;
	struct vm_area_struct *mmap;
	uint asid;

	int pf_num;
	struct list_head fifo;
};

extern void init_page_pool();
void test_page_fault(int val);

struct mm_struct *create_mm_struct(uint asid);
void free_mm_struct(struct mm_struct *mm);
pgd_t *create_pt();

pte_t *get_pte(pgd_t *pgd, void *vaddr);
bool pte_is_valid(pte_t *pte);
u32 get_pte_tlb_entry(pte_t *pte);
void set_pte_tlb_entry(pte_t *pte, u32 value);

u32 get_entry_hi(void *vaddr, uint asid);

void do_pagefault(void *vaddr);
void *swap_one_page(struct mm_struct *mm, void *vaddr);
// void swap_in_one_page(void *vaddr);

int find_vma(struct mm_struct *mm, void *vaddr);

void *mmap(const char *file_name);

#endif // OUROS_VM_H