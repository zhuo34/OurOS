#ifndef REGS_H
#define REGS_H

#include <ouros/type.h>

struct __EntryHi {
    unsigned int ASID : 8;
    unsigned int reserved : 5;
    unsigned int VPN2 : 19;
};

union EntryHi {
	struct __EntryHi reg;
	u32 value;
};

struct __EntryLo {
    unsigned int G : 1;
    unsigned int V : 1;
    unsigned int D : 1;
    unsigned int C : 3;
    unsigned int PFN : 24;
    unsigned int reserved : 2;
};

union EntryLo {
	struct __EntryLo reg;
	u32 value;
};

// typedef struct {
//     unsigned int reserved1 : 12;
//     unsigned int Mask : 16;
//     unsigned int reserved0 : 4;
// } __PageMask;

#endif // REGS_H