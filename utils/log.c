#include <assert.h>
#include <os/log.h>
#include <os/time.h>

int log(int status, const char *format, ...) {
    // print status
    if(status == LOG_OK)
        kernel_printf_color(VGA_GREEN,	VGA_BLACK, "[ O K ] ");
    else if(status == LOG_FAIL)
        kernel_printf_color(VGA_RED,	VGA_BLACK, "[FAIL] ");
    else if(status == LOG_START)
		kernel_printf_color(VGA_BLUE, 	VGA_BLACK, "[START] ");
	else if(status == LOG_END)
		kernel_printf_color(VGA_BLUE, 	VGA_BLACK, "[ END ] ");
	else if(status == LOG_STEP)
		kernel_printf_color(VGA_WHITE, 	VGA_BLACK, "[STEP] ");
	else
		kernel_assert(0, "[LOG]: Undefined log status.");

    // print time
	char time_buf[9];
    get_time(time_buf, sizeof(time_buf));
    kernel_printf(" %s ", time_buf);
	
    // print log message
    va_list argList;
    va_start(argList, format);
    int cnt = kernel_printf_argList(VGA_WHITE, VGA_BLACK, format, argList);
    va_end(argList);
    kernel_printf("\n");

    return cnt;
}
