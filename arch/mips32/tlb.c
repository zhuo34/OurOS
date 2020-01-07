#include "tlb.h"
#include "arch.h"
#include "exc.h"
#include "intr.h"

#include <ouros/utils.h>
#include <driver/vga.h>
#include <ouros/vm.h>
#include <ouros/pc.h>
#include <ouros/mm.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

void init_tlb() {
	asm volatile(
		"mtc0 $zero, $2\n\t"  // entry_lo0
		"mtc0 $zero, $3\n\t"  // entry_lo1
		"mtc0 $zero, $5\n\t"  // PageMask
		"mtc0 $zero, $6\n\t"  // wired register
		"lui  $t0, 0x8000\n\t"
		"li $t1, 0x2000\n\t"

		"move $v0, $zero\n\t"
		"move $t7, $zero\n\t"
		"li $v1, 32\n"

		"init_tlb_L1:\n\t"
		"mtc0 $v0, $0\n\t"
		"mtc0 $t0, $10\n\t"
		"addu $t0, $t0, $t1\n\t"
		"addi $v0, $v0, 1\n\t"
		"ehb\n\t"
		"tlbwi\n\t"
		"bne $v0, $v1, init_tlb_L1\n\t"
		"nop"
	);
	register_exception_handler(2, tlb_read_handler);
	register_exception_handler(3, tlb_write_handler);
}

void print_tlb()
{
	for (int i = 0; i < 32; i++) {
		uint lo0, lo1, hi;
		asm volatile(
			"mtc0 %3, $0\n\t"	// i -> Index
			"tlbr\n\t"
			"mfc0 %0, $2\n\t"	// output EntryLo0
			"mfc0 %1, $3\n\t"	// output EntryL01
			"mfc0 %2, $10\n\t"	// output EntryHi
			"nop"
			: "=r"(lo0), "=r"(lo1), "=r"(hi)
			: "r"(i)
		);
		kernel_printf("tlb %d: %x\t%x\t%x", i, hi, lo0, lo1);
		if (i % 2) {
			kernel_printf("\n");
		} else {
			kernel_printf("\t");
		}
	}
}

void print_tlb_index(uint index)
{
	uint lo0, lo1, hi;
	asm volatile(
		"mtc0 %3, $0\n\t"	// index -> Index
		"tlbr\n\t"
		"mfc0 %0, $2\n\t"	// output EntryLo0
		"mfc0 %1, $3\n\t"	// output EntryL01
		"mfc0 %2, $10\n\t"	// output EntryHi
		"nop"
		: "=r"(lo0), "=r"(lo1), "=r"(hi)
		: "r"(index)
	);
	kernel_printf("tlb %d: %x\t%x\t%x\n", index, hi, lo0, lo1);
}

void tlb_refill()
{
	int old = disable_interrupts();
	__tlb_refill(true);
	if (old)
		enable_interrupts();
}

/**
 * @brief TLB refill
 */
void __tlb_refill(bool random)
{
	uint bad_vaddr, entrylo0, entrylo1;
	pte_t *pte;
	asm volatile(
		"mfc0 %0, $8\n\t"			// load BadVAddr
		: "=r"(bad_vaddr)
	);

	/* Get pte. */
	pte = get_pte(mm_current->pgd, (void *)bad_vaddr);

	/* Load page table information. */
	if (!addr_bind_to_lo1(bad_vaddr)) {
		entrylo0 = get_pte_tlb_entry(pte);
		entrylo1 = get_pte_tlb_entry(pte + 1);
	} else {
		entrylo0 = get_pte_tlb_entry(pte - 1);
		entrylo1 = get_pte_tlb_entry(pte);
	}

	if (pte_is_valid(pte)) {
		// 1. if this entry is valid, refill tlb
		asm volatile(
			"mtc0 %0, $2\n\t"			// write EntryLo0
			"mtc0 %1, $3\n\t"			// write EntryLo1
			"ehb\n\t"					// CP0 hazard
			:
			: "r"(entrylo0), "r"(entrylo1)
		);
		if (random) {
			asm volatile (
				"tlbwr\n\t"
			);
		} else {
			asm volatile (
				"tlbwi\n\t"
			);
		}
	} else {
		// 2. page fault
		do_pagefault((void *) bad_vaddr);
	}
}

void tlb_read_handler(unsigned int status, unsigned int cause, context* context)
{
	// context->epc -= 4;
	uint index;
	asm volatile(
		"tlbp\n\t"
		"mfc0 %0, $0\n\t"			// output index
		: "=r" (index)
	);

	if (index >> 31) {
		// 1. TLB miss
		tlb_refill();
	} else {
		// 2. TLB invalid
		__tlb_refill(false);
	}
}

void tlb_write_handler(unsigned int status, unsigned int cause, context* context)
{
	// context->epc -= 4;
	uint index;
	asm volatile(
		"tlbp\n\t"
		"mfc0 %0, $0\n\t"			// output index
		: "=r" (index)
	);

	if (index >> 31) {
		// 1. TLB miss
		tlb_refill();
	} else {
		// 2. TLB invalid
		__tlb_refill(false);
	}
}

void tlb_reset(struct mm_struct *mm, void *vaddr)
{
	uint entry_hi = get_entry_hi(vaddr, mm->asid);
	asm volatile(
		"mtc0 %0, $10\n\t"			// write EntryHi
		"ehb\n\t"					// CP0 hazard
		:
		: "r"(entry_hi)
	);
	uint index;
	asm volatile(
		"tlbp\n\t"
		"mfc0 %0, $0\n\t"			// output index
		: "=r" (index)
	);

	if (index >> 31) {
		/**
		 * No tlb entry for this address
		 */
		return;
	}

	asm volatile(
		"tlbr\n\t"
	);

	pte_t *pte = get_pte(mm->pgd, vaddr);
	uint entrylo = get_pte_tlb_entry(pte);

	if (!addr_bind_to_lo1(vaddr)) {
		asm volatile(
			"mtc0 %0, $2\n\t"			// write EntryLo0
			:
			: "r"(entrylo)
		);
	} else {
		asm volatile(
			"mtc0 %0, $3\n\t"			// write EntryLo1
			:
			: "r"(entrylo)
		);
	}
	asm volatile(
		"ehb\n\t"					// CP0 hazard
		"tlbwi\n\t"
	);
}

#pragma GCC pop_options
