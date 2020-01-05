#ifndef PAGE_H
#define PAGE_H

#include <regs.h>
#include <ouros/pc.h>
#include <ouros/vm.h>

#define addr_bind_to_lo1(addr) (((uint) addr << 19) >> 31)

/**
 * **************
 * +  MIPS TLB  +
 * **************
 * MIPS tlb has 2 page table entry per tlb entry.
 * Simplified struture:
 *        ++++++++++++++++++++++++++
 *        |  VPN 19 bits  |  ASID  |
 *        ++++++++++++++++++++++++++
 *        |  PFN0  |  flags  |
 *        ++++++++++++++++++++
 *        |  PFN1  |  flags  |
 *        ++++++++++++++++++++
 * In vm.h, we should clear that for 4 K page size,
 * VPN is supposed to be 20 bits. Here VPN has 19 bits
 * because 1 bit is used for distinguish PFN0 and PFN1.
 * 
 * In CP0, EntryLo0, EntryLo1, EntryHi are most related to TLB.
 * EntryLo0/1:
 *        ++++++++++++++++++++++++
 *        | 0 |  PFN   |  flags  |
 *        ++++++++++++++++++++++++
 * EntryHi:
 *        ++++++++++++++++++++++++++++++++
 *        |  VFN 19 bits  | 0 s |  ASID  |
 *        ++++++++++++++++++++++++++++++++
 * 
 * When tlb miss, MIPS will fill the EntryHi with VPN and ASID,
 * the BadVAddr with complete address.
 * 
 * Instruction 'tlbp' use EntryHi to search tlb,
 * set CP0 Index to real index if succeed, set -1 if fail.
 * 
 * Instruction 'tlbwi/r' use EntryHi, EntryLo0, EntryLo1
 * to fill tlb. 'tlbwi' write tlb with Index, 'tlbwr' write randomly.
 */

struct tlb_entry {
    union EntryLo lo0;
    union EntryLo lo1;
    union EntryHi hi;
};

void init_tlb();

void print_tlb();
void print_tlb_index(uint index);

void tlb_refill();
void __tlb_refill(bool random);
void tlb_read_handler(unsigned int status, unsigned int cause, context* context);
void tlb_write_handler(unsigned int status, unsigned int cause, context* context);

void tlb_reset(struct mm_struct *mm, void *vaddr);

#endif // TLB_H
