#ifndef INTR_H
#define INTR_H

#include <ouros/pc.h>

typedef void (*intr_fn)(unsigned int, unsigned int, context*);

extern intr_fn interrupts[8];

void init_interrupts();
int enable_interrupts();
int disable_interrupts();
void do_interrupts(unsigned int status, unsigned int cause, context* sp);
void register_interrupt_handler(int index, intr_fn fn);

#endif // INTR_H
