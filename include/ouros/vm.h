#ifndef OUROS_VM_H
#define OUROS_VM_H

#include <ouros/type.h>
#include <regs.h>
#include <ouros/list.h>

#define PGD_NUM (1 << 10)
#define PTE_NUM (1 << 10)
#define MAX_PFN_PER_PROCESS 3

struct __pte_t {
	union EntryLo tlb_entry;
	u32 info;
};
typedef struct __pte_t pte_t;
typedef struct __pte_t* pgd_t;

extern struct mm_struct *mm_current;

void test_tlb_refill();

struct vm_area_struct {

};

struct mm_struct {
	int count;
	pgd_t *pgd;
	struct vm_area_struct *mmap;

	int pf_num;
	struct list_head fifo;
};

struct mm_struct *create_mm_struct();
pgd_t *create_pt();
void init_pt(pgd_t *pgd);

pte_t *get_pte(pgd_t *pgd, void *vaddr);
bool pte_is_valid(pte_t *pte);
u32 get_pte_tlb_entry(pte_t *pte);
void set_pte_tlb_entry(pte_t *pte, u32 value);

u32 get_entry_hi(void *vaddr);

void do_pagefault(void *vaddr);
void *swap_one_page(struct mm_struct *mm, void *vaddr);
// void swap_in_one_page(void *vaddr);

int find_vma(struct mm_struct *mm, void *vaddr);

#endif // OUROS_VM_H