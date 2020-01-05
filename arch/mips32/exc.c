#include "exc.h"

#include <driver/vga.h>
#include <ouros/pc.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

exc_fn exceptions[32];

void do_exceptions(unsigned int status, unsigned int cause, context* pt_context) {
    int old = disable_interrupts();
    int index = cause >> 2;
    index &= 0x1f;
	
    if (exceptions[index]) {
        exceptions[index](status, cause, pt_context);
    } else {
        kernel_printf("exceptions[%d] %x\n", index, exceptions[index]);
        kernel_printf("EPC in EXC: 0x%x\n", pt_context->epc);
        kernel_printf("EPC inst: %x\n", *(uint*)(pt_context->epc));
        kernel_printf("do exception %d\n", index);
        kernel_printf("status %x\n", status);
        kernel_printf("cause %x\n", cause);
        kernel_printf("No matched handler!\n");        
        while (1)
            ;
    }
    if (old)
        enable_interrupts();
}

void register_exception_handler(int index, exc_fn fn) {
    index &= 31;
    // kernel_printf("register exeception %d, %x\n", index, fn);
    exceptions[index] = fn;
    // kernel_printf("exceptions[%d] %x\n", index, exceptions[index]);
}

void init_exception() {
    // status 0000 0000 0000 0000 0000 0000 0000 0000
    // cause 0000 0000 1000 0000 0000 0000 0000 0000
    asm volatile(
        "mtc0 $zero, $12\n\t"
        "li $t0, 0x800000\n\t"
        "mtc0 $t0, $13\n\t");
    for (int i = 0; i < 32; i++) {
        exceptions[i] = nullptr;
    }
    // register_excepti     on_handler(10, do_exception_10);
}

void do_exception_10(unsigned int status, unsigned int cause, context* pt_context)
{

}

#pragma GCC pop_options