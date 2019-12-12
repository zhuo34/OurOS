#include "page.h"
#include "arch.h"

#include <ouros/utils.h>
#include <driver/vga.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

void init_tlb() {
	asm volatile(
		"mtc0 $zero, $2\n\t"  //entry_lo0
		"mtc0 $zero, $3\n\t"  //entry_lo1
		"mtc0 $zero, $5\n\t"  //PageMask
		"mtc0 $zero, $6\n\t"  //wired register
		"lui  $t0, 0x8000\n\t"
		"li $t1, 0x2000\n\t"

		"move $v0, $zero\n\t"
		"li $v1, 32\n"

		"init_tlb_L1:\n\t"
		"mtc0 $v0, $0\n\t"
		"mtc0 $t0, $10\n\t"
		"addu $t0, $t0, $t1\n\t"
		"addi $v0, $v0, 1\n\t"
		"ehb\n\t"
		"tlbwi\n\t"
		"nop\n\t"
		"bne $v0, $v1, init_tlb_L1\n\t"
		"nop"
	);
	register_exception_handler(2, tlb_read_handler);
	register_exception_handler(3, tlb_write_handler);
}

void read_tlb()
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

void read_tlb_index(uint index)
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
	kernel_printf("begin tlb refill\n");
	uint status, cause;
	asm volatile(
		"mfc0 %0, $12\n\t"			// output Status
		"mfc0 %1, $13\n\t"			// output Cause
		: "=r" (status), "=r" (cause) 
	);
	kernel_printf("Status: %x\n", status);
	kernel_printf("Cause: %x\n", cause);

	asm volatile(
		"lui $k1, %hi(pgd_current)\n\t"
		"mfc0 $k0, $8\n\t"			// load BadVAddr
		"lw $k1, %lo(pgd_current) ($k1)\n\t"
		"srl $k0, $k0, 22\n\t"
		"sll $k0, $k0, 2\n\t"		// get PGD offset
		"addu $k1, $k1, $k0\n\t"

		"mfc0 $k0, $8\n\t"			// load BadVAddr
		"lw $k1, 0($k1)\n\t"		// load PTE point
		"srl $k0, $k0, 10\n\t"
		"andi $k0, $k0, 0xff8\n\t"
		"addu $k1, $k1, $k0\n\t"

		"lw $k0, 0($k1)\n\t"
		"lw $k1, 4($k1)\n\t"
		"mtc0 $k0, $2\n\t"			// write EntryLo0
		"mtc0 $k1, $3\n\t"			// write EntryLo1
		"ehb\n\t"					// CP0 hazard
		"tlbwr\n\t"
		"nop\n\t"
	);
	uint badvaddr, pgd;
	asm volatile(
		"mfc0 %0, $8\n\t"			// output BadVAddr
		: "=r" (badvaddr)
	);
	kernel_printf("BadVAddr: %x\n", badvaddr);
	uint index, hi;
	asm volatile(
		"tlbp\n\t"
		"mfc0 %0, $0\n\t"			// output index
		"mfc0 %1, $10\n\t"			// output hi
		: "=r" (index), "=r" (hi)
	);
	kernel_printf("Index: %x\n", index);
	kernel_printf("EntryHi: %x\n", hi);
	if ((index >> 31) == 0) {
		read_tlb_index(index);
	}
	// kernel_printf("pgd: %x\n", pgd);
	kernel_printf("end tlb refill\n");
}

void tlb_read_handler(unsigned int status, unsigned int cause, context* context)
{
	kernel_printf("tlb_read_handler\n");
	uint index, hi;
	asm volatile(
		"tlbp\n\t"
		"mfc0 %0, $0\n\t"			// output index
		"mfc0 %1, $10\n\t"			// output hi
		: "=r" (index), "=r" (hi)
	);
    kernel_printf("Index: %x\n", index);
	kernel_printf("EntryHi: %x\n", hi);
	if (index >> 31) {
		tlb_refill();
	} else {
		asm volatile(
			"lui $k1, %hi(pgd_current)\n\t"
			"mfc0 $k0, $8\n\t"			// load BadVAddr
			"lw $k1, %lo(pgd_current) ($k1)\n\t"
			"srl $k0, $k0, 22\n\t"
			"sll $k0, $k0, 2\n\t"		// get PGD offset
			"addu $k1, $k1, $k0\n\t"

			"mfc0 $k0, $8\n\t"			// load BadVAddr
			"lw $k1, 0($k1)\n\t"		// load PTE point
			"srl $k0, $k0, 10\n\t"
			"andi $k0, $k0, 0xff8\n\t"
			"addu $k1, $k1, $k0\n\t"

			"lw $k0, 0($k1)\n\t"
			"lw $k1, 4($k1)\n\t"
			"mtc0 $k0, $2\n\t"			// write EntryLo0
			"mtc0 $k1, $3\n\t"			// write EntryLo1
			"ehb\n\t"					// CP0 hazard
			"tlbwi\n\t"
		);
	}
}

void tlb_write_handler(unsigned int status, unsigned int cause, context* context)
{
	kernel_printf("tlb_write_handler\n");
	uint index, hi;
	asm volatile(
		"tlbp\n\t"
		"mfc0 %0, $0\n\t"			// output index
		"mfc0 %1, $10\n\t"			// output hi
		: "=r" (index), "=r" (hi)
	);
    kernel_printf("Index: %x\n", index);
	kernel_printf("EntryHi: %x\n", hi);
	if (index >> 31) {
		tlb_refill();
	} else {
		asm volatile(
			"lui $k1, %hi(pgd_current)\n\t"
			"mfc0 $k0, $8\n\t"			// load BadVAddr
			"lw $k1, %lo(pgd_current) ($k1)\n\t"
			"srl $k0, $k0, 22\n\t"
			"sll $k0, $k0, 2\n\t"		// get PGD offset
			"addu $k1, $k1, $k0\n\t"

			"mfc0 $k0, $8\n\t"			// load BadVAddr
			"lw $k1, 0($k1)\n\t"		// load PTE point
			"srl $k0, $k0, 10\n\t"
			"andi $k0, $k0, 0xff8\n\t"
			"addu $k1, $k1, $k0\n\t"

			"lw $k0, 0($k1)\n\t"
			"lw $k1, 4($k1)\n\t"
			"mtc0 $k0, $2\n\t"			// write EntryLo0
			"mtc0 $k1, $3\n\t"			// write EntryLo1
			"ehb\n\t"					// CP0 hazard
			"tlbwi\n\t"
		);
	}
}

#pragma GCC pop_options