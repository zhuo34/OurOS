#ifndef OUROS_VM_H
#define OUROS_VM_H

#include <ouros/type.h>

extern uint **pgd_current;

void test_tlb_refill();

#endif // OUROS_VM_H