
#include <arch.h>
#include <os/syscall.h>
#include <driver/vga.h>

<<<<<<< HEAD
void syscall4(unsigned int status, unsigned int cause, context* pt_context)
{
    kernel_printf("syscall4 addr%x:\n", pt_context->a0);
    kernel_puts((unsigned char*)pt_context->a0,0xfff,0);
=======
void syscall4(unsigned int status, unsigned int cause, context* pt_context) {
    kernel_printf("syscall4  addr%x:\n", pt_context->a0);
    kernel_putstring((unsigned char*)pt_context->a0,0xfff,0);
>>>>>>> 0b018aa7ee04f2c3d2d739b3b77a81b10cd49ec9
}