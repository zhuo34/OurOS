#ifndef ASSERT_H
#define ASSERT_H

#include <driver/vga.h>

#undef assert
void kernel_assert(int statement, char * message);

#endif // ASSERT_H