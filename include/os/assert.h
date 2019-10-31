#ifndef ASSERT_H
#define ASSERT_H

#include <driver/vga.h>

#undef assert

int kernel_assert(bool statement, const char *format, ...);

#endif // ASSERT_H