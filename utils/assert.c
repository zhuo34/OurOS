#include <assert.h>

int kernel_assert(int statement, const char *format, ...) {
    int cnt = 0;
    if (statement != 1) {
        kernel_printf_error("[ASSERT ERROR]: %s\n");

        va_list argList;
        va_start(argList, format);
        cnt = kernel_printf_argList(VGA_RED, VGA_BLACK, format, argList);
        va_end(argList);

        while (1)
            ;
    }
    return cnt;
}
