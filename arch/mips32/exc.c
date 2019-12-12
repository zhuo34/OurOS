#include "exc.h"

#include <driver/vga.h>
#include <ouros/pc.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

exc_fn exceptions[32];

void do_exceptions(unsigned int status, unsigned int cause, context* pt_context) {
    int index = cause >> 2;
    index &= 0x1f;
    uint badvaddr, pgd;
	// asm volatile(
	// 	"mfc0 %0, $8\n\t"			// output BadVAddr
	// 	: "=r" (badvaddr)
	// );
    // kernel_printf("BadVAddr: %x\n", badvaddr);
    if (exceptions[index]) {
        exceptions[index](status, cause, pt_context);
    } else {
        kernel_printf("do exception %d\n", index);
        kernel_printf("status %x\n", status);
        kernel_printf("cause %x\n", cause);
        kernel_printf("No matched handler!\n");        
        while (1)
            ;
    }
}

void register_exception_handler(int index, exc_fn fn) {
    index &= 31;
    exceptions[index] = fn;
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
}

#pragma GCC pop_options