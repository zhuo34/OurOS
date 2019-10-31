#include "terminal.h"
#include <driver/vga.h>

void clearScreen()
{
    kernel_clear_screen();
}
