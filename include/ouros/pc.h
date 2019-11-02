#ifndef OUROS_PC_H
#define OUROS_PC_H

#include <ouros/type.h>

typedef struct regs_context {
    u32 epc;
    u32 at;
    u32 v0, v1;
    u32 a0, a1, a2, a3;
    u32 t0, t1, t2, t3, t4, t5, t6, t7;
    u32 s0, s1, s2, s3, s4, s5, s6, s7;
    u32 t8, t9;
    u32 hi, lo;
    u32 gp, sp, fp, ra;
}context;

void init_pc();

#endif  // OUROS_PC_H