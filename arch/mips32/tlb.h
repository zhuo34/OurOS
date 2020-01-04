#ifndef PAGE_H
#define PAGE_H

#include <regs.h>
#include <ouros/pc.h>
#include <ouros/vm.h>

#define addr_bind_to_lo1(addr) (((uint) addr << 19) >> 31)

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
