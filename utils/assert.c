#include <assert.h>

int kernel_assert(bool statement, const char *format, ...) {
    int cnt = 0;
    if (!statement) {
        kernel_printf_error("[ASSERT ERROR]:\n");

        va_list argList;
        va_start(argList, format);
        cnt = kernel_printf_argList(VGA_RED, VGA_BLACK, format, argList);
        va_end(argList);

        while (1)
            ;
    }
    return cnt;
}
