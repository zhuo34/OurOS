
#include <arch.h>
#include <ouros/syscall.h>
#include <driver/vga.h>

void syscall4(unsigned int status, unsigned int cause, context* pt_context) {
    kernel_printf("syscall4  addr%x:\n", pt_context->a0);
    kernel_printf("%s", (unsigned char*)pt_context->a0);
}