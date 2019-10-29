
#include <arch.h>
#include <os/syscall.h>
#include <driver/vga.h>

void syscall4(unsigned int status, unsigned int cause, context* pt_context) {
    kernel_printf("syscall4  addr%x:\n", pt_context->a0);
    kernel_putstring((unsigned char*)pt_context->a0,0xfff,0);
}